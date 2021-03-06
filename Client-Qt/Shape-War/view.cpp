#include "view.h"
/*!
 * \brief View::View constructor
 * \param scene
 * \param ws the webSocket of arena (apart off chat)
 */
View::View(Scene *scene, QWebSocket &ws) : QGraphicsView(scene), ws(ws) {
    this->resize(viewWidth, viewHeight);
    this->setWindowTitle("Shape-War");

    setRenderHint(QPainter::Antialiasing, true);

    // size control
    setMinimumSize(500, 400);
    setMaximumSize(maxViewWidth, maxViewHeight);

    // disable the scroll bar
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setMouseTracking(true); // pretty consumes resources of CPU

    this->self = scene->self;
    this->centerOn(this->self->pos());

    connect(self, SIGNAL(xChanged()), this, SLOT(onSelfPosChanged()));
    connect(self, SIGNAL(yChanged()), this, SLOT(onSelfPosChanged()));

    sendDelayTimer = new QTimer(this);
    connect(sendDelayTimer, SIGNAL(timeout()), this,
            SLOT(sendControlToServer()));
    sendDelayTimer->start(20);

    key_a_pressed = key_d_pressed = key_s_pressed = key_w_pressed = false;
    mouseClicked = false;
    upgradeChoose = -1;

    // to expand the skills (aka. passives) options
    expandBtn = new QPushButton("+", this);
    expandBtn->setGeometry(0, viewHeight - buttonLen, buttonLen, buttonLen);
    expandBtn->setVisible(true);
    isExpanded = false;
    connect(expandBtn, SIGNAL(clicked()), this, SLOT(showUpgrateOptions()));

    // set up passives options
    for (int i = 0, size = self->passiveNames.size(); i < size; i++) {
        properties.push_back(new QPair<QLabel *, QPushButton *>(
            new QLabel(self->passiveNames.at(i), this),
            new QPushButton("+", this)));
    }
    propertyBtnPtrGroup = new QButtonGroup(this);
    for (unsigned int i = 0, size = properties.size(); i < size; i++) {
        QPair<QLabel *, QPushButton *> *property = properties.at(i);

        property->first->setVisible(false);
        property->second->setVisible(false);
        property->first->setEnabled(false);
        property->second->setEnabled(false);

        propertyBtnPtrGroup->addButton(property->second, i);
    }
    this->setPropertyStyle();
    connect(self, SIGNAL(upgradePointsChanged()), this,
            SLOT(onUpgradePointChanged()));
    connect(propertyBtnPtrGroup, SIGNAL(buttonClicked(int)), this,
            SLOT(onPropertyBtnClicked(int)));

    // timer for testing tansmittion rate
    sec = new QTimer(0);
    this->sends = 0;
    connect(sec, SIGNAL(timeout()), this, SLOT(print_freq()));
    sec->start(1000);

    // new chat bar
    this->chatBar = new ChatBar(scene->getPartUrl(), this);
    this->chatBar->setName(scene->getUsername());

    // when passives changes do necessary display update
    connect(self, SIGNAL(passiveChanged(int, int)), this,
            SLOT(onPassivesChanged(int, int)));

    // set up reborn interface
    this->rebornLabel = new QLabel("Going to reborn?", this);
    this->rebornLabel->setGeometry(this->width() / 2 - 150,
                                   this->height() * 2 / 3, 300, 50);
    this->rebornLabel->setVisible(false);
    this->rebornBtn = new QPushButton(this);
    this->rebornBtn->setGeometry(this->width() / 2 - 30, this->height() * 2 / 3 + 50,
                                 60, 40);
    this->rebornBtn->setVisible(false);
    this->rebornBtn->setText("Reborn");
    this->rebornChoose = false;

    this->rebornLabel->setStyleSheet("font: 40px");
    this->rebornBtn->setStyleSheet("background-color: rgba(255, 153, 0, 200) ;"
                                   "border-style: outset; "
                                   "border-color: rgba(255, 153, 0, 200); border-top-left-radius: 15px;"
                                   "border-bottom-right-radius: 15px;"
                                   "border-top-right-radius: 15px;"
                                   "border-bottom-left-radius: 15px;"
                                   );
    connect(self->hpBar, SIGNAL(dieSignal()), this, SLOT(onSelfDie()));
    connect(rebornBtn, SIGNAL(clicked(bool)), this,
            SLOT(onRebornClicked(bool)));
}

/*!
 * \brief View::print_freq print the rating
 */
void View::print_freq() {
    qDebug() << "send per sec: " << this->sends;
    this->sends = 0;
}

/*!
 * \brief View::onSelfDie update display setting when die
 */
void View::onSelfDie() {
    this->rebornBtn->setVisible(true);
    this->rebornLabel->setVisible(true);
    // this->setStyleSheet("background-color: rgb(61, 61, 61, 240);");
    for (QObject *child : this->children()) {
        QWidget *childWidget = dynamic_cast<QWidget *>(child);
        if (childWidget == NULL)
            continue;
    }
}

/*!
 * \brief View::onRebornClicked update display setting when click reborn button
 */
void View::onRebornClicked(bool) {
    this->rebornBtn->setVisible(false);
    this->rebornLabel->setVisible(false);
    this->rebornChoose = true;
}

/*!
 * \brief View::keyPressEvent collect the control actions of user
 * \param event
 */
void View::keyPressEvent(QKeyEvent *event) {
    // ment is just for testing
    // whether to move to judged by server instead of here

    switch (event->key()) {
    case Qt::Key_W:
        this->key_w_pressed = true;
        break;
    case Qt::Key_A:
        this->key_a_pressed = true;
        break;
    case Qt::Key_S:
        this->key_s_pressed = true;
        break;
    case Qt::Key_D:
        this->key_d_pressed = true;
        break;
    }
}

/*!
 * \brief View::keyReleaseEvent collect the control actions of users
 * \param event
 */
void View::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }
    switch (event->key()) {
    case Qt::Key_W:
        this->key_w_pressed = false;
        break;
    case Qt::Key_A:
        this->key_a_pressed = false;
        break;
    case Qt::Key_S:
        this->key_s_pressed = false;
        break;
    case Qt::Key_D:
        this->key_d_pressed = false;
        break;
    case Qt::Key_Enter:
        this->chatBar->startChat();
        break;
    case Qt::Key_Return:
        this->chatBar->startChat();
        break;
    }
    // qDebug() << "Released key: " << event->key();
}

/*!
 * \brief View::onSelfPosChanged center on the view and setting rotation and info position of self
 */
void View::onSelfPosChanged() {
    this->centerOn(this->self->pos());
    QPointF mouseP = this->mapToScene(this->mapFromGlobal(QCursor::pos()));
    self->setRotation(this->calcRargetAngle(mouseP));
    self->setInfoPos(this->mapToScene(QPoint(InfoCenterX, InfoCenterY)));
}

/*!
 * \brief View::mousePressEvent collect control action of user
 * \param event
 */
void View::mousePressEvent(QMouseEvent *event) {
    (void)event;
    mouseClicked = true;
}

/*!
 * \brief View::mouseReleaseEvent collect control action of user
 * \param event
 */
void View::mouseReleaseEvent(QMouseEvent *event) {
    (void)event;
    mouseClicked = false;
}

/*!
 * \brief View::mouseMoveEvent collect control action of user
 * \param event
 */
void View::mouseMoveEvent(QMouseEvent *event) {
    QPointF mouseP = this->mapToScene(event->pos());
    self->setRotation(this->calcRargetAngle(mouseP));
    // printf("self (%f, %f), mouse (%f,%f). degree of mouse relative to self
    // is: %f",selfP.x(), selfP.y(), mouseP.x(), mouseP.y(),
    //                  targetAngle * 180.0 / 3.14 );
}

/*!
 * \brief View::calcRargetAngle calclate the angle between mouse and self
 * \param mouseP
 * \return
 */
qreal View::calcRargetAngle(QPointF &mouseP) {
    QPointF selfP = self->pos();
    qreal tangent = (mouseP.y() - selfP.y()) / (mouseP.x() - selfP.x());
    qreal targetAngle = atan(tangent);
    if (mouseP.x() < selfP.x())
        targetAngle += M_PI;
    else if (mouseP.y() < selfP.y())
        targetAngle += M_PI * 2;
    targetAngle *= 180;
    targetAngle /= 3.14;
    return targetAngle;
}
/*!
 * \brief View::wheelEvent eat this event to prevent it is passed to parent widget
 */
void View::wheelEvent(QWheelEvent *) {
    // do nothing
}

/*!
 * \brief View::resizeEvent when the view is resized set necessary position of objects in view
 * \param event
 */
void View::resizeEvent(QResizeEvent *event) {
    (void)event;
    int nowViewWidth = this->width();
    int nowViewHeight = this->height();
    if (isExpanded) {
        expandBtn->setText("-");
        expandBtn->setGeometry(
            0, nowViewHeight - (properties.size() + 1) * passiveDistance - 15,
            buttonLen, buttonLen);
    } else {
        expandBtn->setText("+");
        expandBtn->setGeometry(0, nowViewHeight - buttonLen, buttonLen,
                               buttonLen);
    }
    for (unsigned int i = 0, size = properties.size(); i < size; i++) {
        QPair<QLabel *, QPushButton *> *property = properties.at(i);
        property->first->setGeometry(10,
                                     nowViewHeight - (i + 1) * passiveDistance,
                                     labelWidth, passiveHeight);
        property->second->setGeometry(labelWidth + 10,
                                      nowViewHeight - (i + 1) * passiveDistance,
                                      buttonLen + 10, passiveHeight);
    }
    InfoCenterX = nowViewWidth / 2;
    InfoCenterY = nowViewHeight - InfoHeightOffset;
    this->onSelfPosChanged();
    this->chatBar->setParentWidth();
    this->rebornBtn->setGeometry(this->width() / 2 - 30, this->height() * 2 / 3 + 50,
                                 60, 40);
    this->rebornLabel->setGeometry(this->width() / 2 - 150,
                                   this->height() * 2 / 3, 300, 50);
}

/*!
 * \brief View::sendControlToServer send the collected control actions to server and reset some of the,
 */
void View::sendControlToServer() {
    this->sends++;
    QJsonObject data;
    data["keys"] = QJsonObject({{"W", key_w_pressed},
                                {"A", key_a_pressed},
                                {"S", key_s_pressed},
                                {"D", key_d_pressed}});
    data["mouse"] = mouseClicked;
    data["angle"] = self->rotation();
    data["upChoose"] = upgradeChoose;
    data["reborn"] = rebornChoose;
    ws.sendTextMessage(QJsonDocument(data).toJson(QJsonDocument::Compact));

    // reset upgrate choose to -1
    upgradeChoose = -1;
    rebornChoose = false;
}

/*!
 * \brief View::showUpgrateOptions show or hide the upgrade options
 */
void View::showUpgrateOptions() {
    isExpanded = !isExpanded;
    if (isExpanded) {
        expandBtn->setText("-");
        expandBtn->setGeometry(
            0, this->height() - (properties.size() + 1) * passiveDistance - 15,
            buttonLen, buttonLen);
        for (QPair<QLabel *, QPushButton *> *property : properties) {
            property->first->setVisible(true);
            property->second->setVisible(true);
        }
    } else {
        expandBtn->setText("+");
        expandBtn->setGeometry(0, this->height() - buttonLen, buttonLen,
                               buttonLen);
        for (QPair<QLabel *, QPushButton *> *property : properties) {
            property->first->setVisible(false);
            property->second->setVisible(false);
        }
    }
}

/*!
 * \brief View::onUpgradePointChanged setting display of upgrade option when upgrade point changed
 */
void View::onUpgradePointChanged() {
    printf("now UP: %d", self->getUpgradePoints());
    bool enabled = (self->getUpgradePoints() > 0) ? true : false;
    for (int i = 0, size = properties.size(); i < size; i++) {
        if (self->getPassiveLevel(i) >= self->passiveMax)
            continue;
        QPair<QLabel *, QPushButton *> *property = properties.at(i);
        property->first->setEnabled(enabled);
        property->second->setEnabled(enabled);
    }
    if (enabled == true && isExpanded == false) {
        this->showUpgrateOptions();
    }
    this->setPropertyStyle();
}

/*!
 * \brief View::onPropertyBtnClicked collect control action of upgrade
 * \param clickedBtnId
 */
void View::onPropertyBtnClicked(int clickedBtnId) {
    qDebug() << "Button clicked: " << this->self->passiveNames.at(clickedBtnId);
    // TODO: handle the message want to  pass to server
    upgradeChoose = clickedBtnId;
    this->setPropertyStyle();
}

/*!
 * \brief View::onPassivesChanged set display option when passives changed
 * \param i
 * \param value
 */
void View::onPassivesChanged(int i, int value) {
    if (value == 0) {
        properties.at(i)->first->setText(self->passiveNames.at(i));
    } else {
        properties.at(i)->first->setText(self->passiveNames.at(i) + " " +
                                         QString::number(value));
    }
    if (value >= self->passiveMax) {
        properties.at(i)->first->setEnabled(false);
        properties.at(i)->second->setEnabled(false);
        this->setPropertyStyle();
    }
}

/*!
 * \brief View::setPropertyStyle draw the style of upgrade options interface
 */
void View::setPropertyStyle() {
    for (int i = 0; i < properties.size(); ++i) {
        this->properties.at(i)->first->setStyleSheet(
            "background-color: rgb(61, 61, 61, 240); border-style: outset; "
            "border-color: rgb(61, 61, 61, 240); border-top-left-radius: "
            "10px; border-bottom-left-radius: 10px; font: bold 14px; "
            "text-align: right; color: rgb(218, 218, 218, 240);");
        this->properties.at(i)->first->setAlignment(Qt::AlignCenter);
    }
    QString common =
        "border-top-right-radius:10px; border-bottom-right-radius: 10px; "
        "font: bold 14px; border-width: 2px; border-style: outset; "
        "border-color: rgb(61, 61, 61, 240); color: rgb(61, 61, 61, 240);";

    if (this->properties.at(0)->second->isEnabled()) {
        this->properties.at(0)->second->setStyleSheet(
            common.append("background-color: rgb(108, 240, 236, 255);"));
    } else {
        this->properties.at(0)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(1)->second->isEnabled()) {
        this->properties.at(1)->second->setStyleSheet(
            common + "background-color: rgb(152, 240, 108, 255);");
    } else {
        this->properties.at(1)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(2)->second->isEnabled()) {
        this->properties.at(2)->second->setStyleSheet(
            common + "background-color: rgb(240, 108, 108, 255);");
    } else {
        this->properties.at(2)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(3)->second->isEnabled()) {
        this->properties.at(3)->second->setStyleSheet(
            common + "background-color: rgb(240, 217, 108, 255);");
    } else {
        this->properties.at(3)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(4)->second->isEnabled()) {
        this->properties.at(4)->second->setStyleSheet(
            common + "background-color: rgb(108, 150, 240, 255);");
    } else {
        this->properties.at(4)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(5)->second->isEnabled()) {
        this->properties.at(5)->second->setStyleSheet(
            common + "background-color: rgb(154, 108, 240, 255);");
    } else {
        this->properties.at(5)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(6)->second->isEnabled()) {
        this->properties.at(6)->second->setStyleSheet(
            common + "background-color: rgb(236, 108, 240, 255);");
    } else {
        this->properties.at(6)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }

    if (this->properties.at(7)->second->isEnabled()) {
        this->properties.at(7)->second->setStyleSheet(
            common + "background-color: rgb(238, 182, 143, 255);");
    } else {
        this->properties.at(7)->second->setStyleSheet(
            common.append("background-color: rgb(156, 156, 156, 255);"));
    }
}

/*!
 * \brief View::setMouseClicked setter of mouseClicked
 * \param value
 */
void View::setMouseClicked(bool value) {
    this->mouseClicked = value;
}
