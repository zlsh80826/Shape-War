#include <hero.h>
#include <QPainter>
#include <QKeyEvent>

Hero::Hero() {
    this -> width = 60;
    this -> setFlags(QGraphicsItem::ItemIsFocusable);
    printf("Hero_constructed");
}

QRectF Hero::boundingRect() const {
    qreal halfPenWidth = 1/2;
    return QRectF( -width/2 - halfPenWidth, -width/2 - halfPenWidth, width + halfPenWidth, width + halfPenWidth);
}

void Hero::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter -> setBrush(QBrush(QColor(0, 178, 255, 255), Qt::SolidPattern));
    painter -> setRenderHint( QPainter::Antialiasing );
    painter -> drawPath(this->shape());
}

QPainterPath Hero::shape() const {
    QPainterPath path;
    path.addRect(-width/2, -width/2, width, width);
    return path;
}

void Hero::setTargetAngle(qreal targetAngle) {
    this->targetAngle = targetAngle;
}

void Hero::read(const QJsonObject &json){
    QJsonObject instance = json["self"].toObject();
    this -> setX(instance["x"].toDouble());
    this -> setY(instance["y"].toDouble());
    this -> setTargetAngle(instance["angle"].toDouble());
    this -> maxHp = instance["maxHp"].toInt();
    this -> currentHp = instance["currentHp"].toInt();
    this -> experience = instance["experience"].toInt();
    this -> level = instance["level"].toInt();

    QJsonArray passivesArray = instance["passives"].toArray();
    for(int i=0; i < passivesArray.size(); ++i){
        this -> passives[i] = passivesArray[i].toInt();
    }
}
