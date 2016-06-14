#ifndef BULLET_H
#define BULLET_H
#include <QGraphicsObject>

class Bullet : public QGraphicsObject {
    Q_OBJECT
public:
    Bullet();
    // need to optimize to boundingRegion
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;
    void setTargetAngle(qreal targetAngle);

private:
    int radius;
    qreal targetAngle;
};
#endif // BULLET_H