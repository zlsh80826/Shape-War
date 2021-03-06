#include "selfinfo.h"
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QTimer>

/*!
 * \brief SelfInfo::SelfInfo constructor
 */
SelfInfo::SelfInfo() {
    // set initial values
    this->score = 0;
    this->targetScore = 0;
    this->maxScore = 50000;
    this->lv = 1;
    this->exp = 0;
    this->max_exp = 10;
    this->targetExp = 0;
    this->expWidth = 0;
    this->maxExpWidth = 500;
    this->scoreWidth = 0;
    this->maxScoreWidth = 400;
    this->setOpacity(0.95);
    this->setZValue(1);

    // new timer and connect signals/slots
    this->expTimer = new QTimer(this);
    this->scoreTimer = new QTimer(this);
    QObject::connect(this->expTimer, SIGNAL(timeout()), this, SLOT(expAni()));
    QObject::connect(this->scoreTimer, SIGNAL(timeout()), this,
                     SLOT(scoreAni()));
}

/*!
 * \brief SelfInfo::setName setter of name
 * \param name
 */
void SelfInfo::setName(QString name) {
    this->name = name;
}

/*!
 * \brief SelfInfo::boundingRect (the same idea of bullet::boudingRect)
 * \return
 */
QRectF SelfInfo::boundingRect() const {
    return QRectF(-380, -40, 760, 160);
}

/*!
 * \brief SelfInfo::paint (the same idea of bullet::paint)
 * \param painter
 * \param option
 * \param widget
 */
void SelfInfo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
    (void)option;
    (void)widget;

    QPen pen;
    pen.setWidth(3);
    painter->setRenderHint(QPainter::Antialiasing);
    pen.setColor(QColor(61, 61, 61, 255));
    painter->setPen(pen);
    painter->setBrush(QBrush(QColor(61, 61, 61, 255), Qt::SolidPattern));
    painter->drawRect(QRect(-maxScoreWidth / 2, 50, maxScoreWidth, 20));
    painter->setBrush(QBrush(QColor(108, 240, 162, 255), Qt::SolidPattern));
    painter->drawRect(QRect(-maxScoreWidth / 2, 50, scoreWidth, 20));
    painter->setBrush(QBrush(QColor(61, 61, 61, 255), Qt::SolidPattern));
    painter->drawRect(QRect(-maxExpWidth / 2, 80, maxExpWidth, 20));
    painter->setBrush(QBrush(QColor(240, 217, 108, 255), Qt::SolidPattern));
    painter->drawRect(QRect(-maxExpWidth / 2, 80, this->expWidth, 20));
    painter->setBrush(QBrush(QColor(240, 240, 240, 255), Qt::SolidPattern));
    QPainterPath namePath;
    namePath.addText(-12 * name.size(), 40, QFont("monospace", 30, QFont::Bold),
                     name);
    painter->drawPath(namePath);
    pen.setWidth(1);
    painter->setPen(pen);
    QPainterPath infoPath;
    infoPath.addText(-8 * lv_str.size(), 97,
                     QFont("monospace", 12, QFont::Expanded), lv_str);
    painter->drawPath(infoPath);

    infoPath.addText(-6 * scr_str.size(), 66,
                     QFont("monospace", 12, QFont::Expanded), scr_str);
    painter->drawPath(infoPath);
}

/*!
 * \brief SelfInfo::setExp setter of exp and max_exp, also control animation processes
 * \param exp
 * \param max_exp
 */
void SelfInfo::setExp(int exp, int max_exp) {
    if (exp == this->targetExp && max_exp == this->max_exp)
        return;
    printf("new_exp: %d, new_max_exp: %d, lv:%d", exp, max_exp, lv);
    this->targetExp = exp;
    this->max_exp = max_exp;
    this->expTimer->start(25);
}

/*!
 * \brief SelfInfo::setMaxScore setter of max score
 * \param value
 */
void SelfInfo::setMaxScore(int value)
{
    maxScore = value;
}

/*!
 * \brief SelfInfo::setScore setter of score
 * \param new_score
 */
void SelfInfo::setScore(int new_score) {

    if (new_score == this->targetScore)
        return;
    this->targetScore = new_score;
    this->scoreTimer->start(25);
    this->scr_str = "Score:  " + QString::number(new_score);
}

/*!
 * \brief SelfInfo::setLv setter of level
 * \param new_lv
 */
void SelfInfo::setLv(int new_lv) {
    if (this->lv == new_lv)
        return;
    this->lv = new_lv;
    this->lv_str = "Lv: " + QString::number(new_lv);
}

/*!
 * \brief SelfInfo::expAni animates the exp bar
 */
void SelfInfo::expAni() {
    if (targetExp == exp) {
        expTimer->stop();
    } else if (targetExp > exp) {
        exp += ((targetExp - exp) / 50 + 1);
    } else if (targetExp < exp) {
        exp -= ((exp - targetExp) / 50 + 1);
    }

    this->expWidth = (this->maxExpWidth * this->exp) / this->max_exp;
    this->update(this->boundingRect());
}

/*!
 * \brief SelfInfo::scoreAni animates the score bar
 */
void SelfInfo::scoreAni() {
    if (this->targetScore == this->score) {
        scoreTimer->stop();
        return;
    } else if (this->targetScore > this->score) {
        score += ((targetScore - score) / 50 + 1);
    } else if (this->targetScore < this->score) {
        score -= ((score - targetScore) / 50 + 1);
    }
    printf("%d / %d", score, maxScore);
    this->scoreWidth = (this->maxScoreWidth * this->score) / this->maxScore;
    this->update(this->boundingRect());
}
