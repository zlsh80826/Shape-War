#include "view.h"

View::View(Scene *scene): QGraphicsView(scene)
{
    viewWidth = 500;
    viewHeight = 500;
    this->resize(viewWidth, viewHeight);
    this->setWindowTitle("Shape-War");

    setRenderHint(QPainter::Antialiasing, true);
    //setCacheMode(CacheBackground);    // I don't know why but this cause unexpected wrong

    // size control, maybe not necessary
    setMinimumSize(viewWidth, viewHeight);
    setMaximumSize(viewWidth, viewHeight);

    this->setMouseTracking(true);  // pretty consumes resources of CPU

    this->self = scene->self;
    this->centerOn(this->self->pos());
}


void View::keyPressEvent(QKeyEvent *event) {
    // movement is just for testing
    // whether to move to judged by server instead of here
    const qreal moveAmount = 3.0;

    switch( event -> key() ){
        case Qt::Key_W :
            this->key_w_pressed = true;
            self->moveBy(0, -moveAmount);
            break;
        case Qt::Key_A :
            this->key_a_pressed = true;
            self->moveBy(-moveAmount, 0);
            break;
        case Qt::Key_S :
            this->key_s_pressed = true;
            self->moveBy(0, moveAmount);
            break;
        case Qt::Key_D :
            this->key_d_pressed = true;
            self->moveBy(moveAmount, 0);
            break;
    }
    this->centerOn(this->self->pos());
}
void View::mousePressEvent(QMouseEvent *event) {
    mouseClicked = true;
}
void View::mouseMoveEvent(QMouseEvent *event) {
    QPointF mouseP = event->pos();
    QPointF selfP = self->pos();
    qreal tangent =  ( mouseP.y() - selfP.y() ) / (mouseP.x() - selfP.x());
    qreal targetAngle = atan( tangent );
    if( mouseP.x() < selfP.x() ) targetAngle += M_PI;
    else if( mouseP.y() < selfP.y() ) targetAngle += M_PI*2;
    self->setTargetAngle( targetAngle );
    //printf("degree of mouse relative to self is: %f", targetAngle * 180.0 / 3.14 );
}
