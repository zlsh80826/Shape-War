from collections import defaultdict
import itertools
import math
import cmath
import random
import logging

from . import abilities
from .motion import MovableObject


logger = logging.getLogger(__name__)


class Hero(abilities.PropertyMixin, MovableObject):

    visible = True
    counter = itertools.count()

    def __init__(self):
        self.skill_points = 0
        self.levels = defaultdict(int)
        self.abilities = abilities.Abilities(self)
        self.id = next(self.counter)

        super().__init__()

        self.radius = 30
        self.acc = 0.6  # acceleration
        self.hp = self.max_hp
        self.experience = 0
        self.level = 1
        self.score = 0

        self.ready_bullets = []
        self.bullets = [Bullet(i, self) for i in range(100)]
        self.ready_bullets.extend(self.bullets)

        self.last_control = {
            'keys': {'W': False, 'A': False, 'S': False, 'D': False},
            'angle': 0,
            'mouse': False,
            'upChoose': -1,
            'reborn': False
        }
        self.cooldown = 0

        self.choose = -1
        self.goReborn = False

    @property
    def angle(self):
        return self.last_control['angle']

    def action(self):
        if not self.visible:
            if not self.goReborn:
                return
            else:
                self.reborn()
        if self.cooldown > 0:
            self.cooldown -= 1
        elif self.last_control['mouse']:
            self.shoot(self.ready_bullets.pop())
            self.cooldown += self.reload
        self.accept_keys(**self.last_control['keys'])
        if self.hp > 0:
            self.hp = min(self.max_hp, self.hp_regen + self.hp)
        if self.choose >= 0:
            self.abilities[self.choose].upgrade()
            self.choose = -1

    def handle_upgrade(self):
        if self.last_control['upChoose'] >= 0:
            self.choose = self.last_control['upChoose']
        if self.last_control['reborn']:
            self.goReborn = True

    def accept_keys(self, W, A, S, D):
        if W or A or S or D:
            self.velocity += cmath.rect(self.acc, math.atan2(S - W, D - A))

    def to_self_dict(self, max_score):
        return {
            'id': self.id,
            'level': self.level,
            'experience': self.experience,
            'max_exp': self.max_exp,
            'passives': [ability.level for ability in self.abilities],
            'upgradePoints': self.skill_points,
            'maxScore': max_score,
            'score': self.score
        }

    def to_player_dict(self):
        return {
            'x': self.x,
            'y': self.y,
            'id': self.id,
            'maxHp': self.max_hp,
            'currentHp': self.hp,
            'angle': self.angle,
            'bullets': [bullet.to_dict() for bullet in self.bullets]
        }

    def shoot(self, bullet):
        bullet.hp = self.bullet_hp
        bullet.body_damage = self.bullet_damage
        bullet.pos = self.pos + self.velocity + cmath.rect(
            self.radius,
            math.radians(self.angle)
        )
        bullet.velocity = cmath.rect(
            self.bullet_speed, math.radians(self.angle))
        bullet.visible = True
        bullet.recycle_timer = 50
        bullet.timeout = 100

    @property
    def max_exp(self):
        return int(10 * (1.2 ** (self.level - 1)))

    def add_exp(self, ammount):
        self.experience += ammount
        self.add_score(ammount)
        while self.experience >= self.max_exp:
            self.experience -= self.max_exp
            self.level += 1
            self.skill_points += 1
            logger.info(
                'level: %d, exp: %r, max_exp: %d',
                self.level,
                self.experience,
                self.max_exp
            )

    def add_score(self, ammount):
        self.score += ammount

    @property
    def team(self):
        return self

    def tick(self):
        if self.visible:
            self.tick_pos()

    @property
    def rewarding_experience(self):
        return self.score // 2

    def killed(self, other):
        self.add_exp(other.rewarding_experience)

    def reborn(self):
        self.visible = True
        self.goReborn = False
        self.hp = self.max_hp

        self.level = 1
        self.skill_points = 0
        self.levels = defaultdict(int)

        self.score = self.score // 2
        self.experience = self.score
        while self.experience >= self.max_exp:
            self.experience -= self.max_exp
            self.level += 1
            self.skill_points += 1

        super().spawn()


class Bullet(MovableObject):

    rewarding_experience = 0

    def __init__(self, id, hero):
        super().__init__()
        self.id = id
        self.radius = 20
        self.angle = 0
        self.visible = False
        self.maxHp = 1
        self.hp = 1
        self.timeout = 0
        self.owner = hero
        self.friction = 0
        self.max_speed = 10000
        self.recycle_timer = 0
        self.x_min = self.y_min = float('-inf')
        self.x_max = self.y_max = float('inf')

    @property
    def team(self):
        return self.owner

    def tick(self):
        if self.visible:
            self.tick_pos()
            self.timeout -= 1
            if self.timeout == 0 or self.hp < 0:
                self.visible = False
        elif self.recycle_timer:
            self.recycle_timer -= 1
        else:
            self.owner.ready_bullets.append(self)

    def to_dict(self):
        return {
            'id': self.id,
            'x': self.x,
            'y': self.y,
            'hp': self.hp,
            'maxHp': self.maxHp,
            'radius': self.radius,
            'visible': self.visible
        }

    def killed(self, other):
        self.owner.killed(other)
