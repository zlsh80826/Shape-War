#ifndef POLYGONGROUP_H
#define POLYGONGROUP_H


template <class Polygon> class PolygonGroup {
private:
    int count;
    QVector<Polygon *> polygons;

public:
    PolygonGroup(int count = 300): count(count) {
        for (int i = 0; i < count; ++i)
            polygons.append(new Polygon);
    }

    void addToParent(QGraphicsScene * parent) {
        for (int i = 0; i < count; ++i) {
            parent->addItem(polygons[i]);
            parent->addItem(polygons[i]->hpBar);
        }
    }

    void read(const QJsonArray &polygonInfo) {
        for (int i = 0; i < polygonInfo.size() and i < count; ++i) {
            polygons[i]->read(polygonInfo[i].toObject());
        }
    }

};

#endif // POLYGONGROUP_H
