import random


class Garbage:

    def __init__(self, id):
        self.id = id
        self.spawn()
        self.visable = False

    def spawn(self):
        self.visable = True
        self.x = random.randrange(self.manager.max_x)
        self.y = random.randrange(self.manager.max_y)

    def to_dict(self):
        return {
            'id': self.id,
            'x': self.x,
            'y': self.y,
            'angle': self.angle,
            'hp': self.hp,
            'maxHp': self.maxHp,
            'radius': self.radius
        }


class Square(Garbage):

    radius = 5
    maxHp = 100


class Triangle(Garbage):

    radius = 5
    maxHp = 400


class Pentagon(Garbage):

    radius = 10
    maxHp = 5000