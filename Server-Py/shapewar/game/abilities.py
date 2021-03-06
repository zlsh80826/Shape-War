import logging


logger = logging.getLogger(__name__)


class UpgradableAbility:

    def __init__(self, name, desc, values):
        self.name = name
        self.desc = desc
        self.values = values

    @property
    def max_level(self):
        return len(self.values) - 1

    def __get__(self, instance, type):
        if type is None:
            return self
        return BoundAbalitiy(self, instance).value

    def __repr__(self):
        return '<%s %r>' % (self.__class__.__name__, self.name)


class BoundAbalitiy:

    def __init__(self, abality, hero):
        self.abality = abality
        self.hero = hero

    def upgrade(self):
        if (
            self.hero.skill_points > 0 and
            self.hero.levels[self.abality] < self.abality.max_level
        ):
            self.hero.skill_points -= 1
            self.hero.levels[self.abality] += 1

    @property
    def value(self):
        return self.abality.values[self.level]

    @property
    def level(self):
        return self.hero.levels[self.abality]

    def __repr__(self):
        return '<BoundAbalitiy %r of %r>' % (self.abality, self.hero)


class Abilities:

    abilities = (
        UpgradableAbility(
            'hp_regen', 'Health Regen',
            [1, 2, 3, 4, 5, 6, 7, 8, 9]
        ),
        UpgradableAbility(
            'max_hp', 'Max Health',
            [1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000]
        ),
        UpgradableAbility(
            'body_damage', 'Body Damage',
            [8, 10, 12, 14, 16, 18, 20, 22, 24]
        ),
        UpgradableAbility(
            'bullet_speed', 'Bullet Speed',
            [8, 10, 12, 14, 16, 18, 20, 22, 24, 26]
        ),
        UpgradableAbility(
            'bullet_hp', 'Bullet Penetration',
            [100, 150, 200, 250, 300, 350, 400, 450, 500]
        ),
        UpgradableAbility(
            'bullet_damage', 'Bullet Damage',
            [4, 5, 6, 7, 8, 9, 10, 11, 12]
        ),
        UpgradableAbility(
            'reload', 'Reload',
            [10, 9, 8, 7, 6, 5, 4, 3, 2]
        ),
        UpgradableAbility(
            'max_speed', 'Movement Speed',
            [7, 8, 9, 10, 11, 12, 13, 14, 15]
        )
    )

    def __init__(self, hero):
        self.hero = hero

    def __getitem__(self, index):
        return BoundAbalitiy(self.abilities[index], self.hero)


def get_property_mixin_namespace():
    namespace = {}
    for abality in Abilities.abilities:
        namespace[abality.name] = abality
    return namespace


PropertyMixin = type(
    'PropertyMixin',
    (),
    get_property_mixin_namespace())
