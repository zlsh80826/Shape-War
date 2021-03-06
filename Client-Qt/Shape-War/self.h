#ifndef SELF_H
#define SELF_H

#include "hero.h"
#include "selfinfo.h"
/*!
 * \brief The Self class provide a interface to show self's information (which is not known to ohter players)
 */
class Self : public Hero {
    Q_OBJECT
public:
    Self();
    void read_info(const QJsonObject &json);
    void read_global(const QJsonObject &json);
    SelfInfo *info;
    const QStringList passiveNames = (QStringList() << "Health Regen"
                                                    << "Max Health"
                                                    << "Body Damage"
                                                    << "Bullet Speed"
                                                    << "Bullet Penetration"
                                                    << "Bullet Damage"
                                                    << "Reload"
                                                    << "Movement Speed");
    void setInfoPos(QPointF);
    int getUpgradePoints() const;
    void setUpgradePoints(int value);
    int getPassiveLevel(int i);
    const int passiveMax = 8;
signals:
    void upgradePointsChanged();
    void passiveChanged(int id, int changeToValue);

private:
    int passives[8];
    int upgradePoints;
};

#endif // SELF_H
