import json
import logging
import itertools
import collections
import random
from tornado.websocket import WebSocketHandler, WebSocketClosedError
from tornado.ioloop import PeriodicCallback

from .hero import Hero, Bullet
from . import garbage
from .collision import bounding_box_collision_pairs, collide


logger = logging.getLogger(__name__)


class Arena:

    tick_time = 33  # tick time in milliseconds

    def __init__(self):
        self.clients = set()

        self.squares = [garbage.Square(i) for i in range(250)]
        self.triangles = [garbage.Triangle(i) for i in range(50)]
        self.pentagons = [garbage.Pentagon(i) for i in range(10)]

        self.tick_id = 0
        self.tick_timer = PeriodicCallback(self.tick, self.tick_time)
        self.pptid = 0
        self.perf_timer = PeriodicCallback(self.perf_callback, 1000)

    def perf_callback(self):
        self.pptid, diff = self.tick_id, self.tick_id - self.pptid
        logger.log(
            logging.INFO
            if diff > 1000 / self.tick_time - 2
            else logging.WARNING,
            '%d clients, %d ticks/s', len(self.clients), diff
        )

    def bind_application(self, application):
        if hasattr(self, 'application'):
            raise RuntimeError(
                'This arena is bounded to %r already' % self.application)
        self.application = application
        self.tick_timer.start()
        self.perf_timer.start()
        logger.info('started arena')

    def iter_all_bullets(self):
        for client in self.clients:
            yield from client.hero.bullets

    def tick(self):
        """this function is called every `self.tick_time` milliseconds
        to process each tick
        """
        for polygon in itertools.chain(
            self.squares,
            self.triangles,
            self.pentagons
        ):
            if polygon.visible:
                polygon.tick_angle()
                polygon.apply_friction()
            else:
                if random.random() < .01:
                    polygon.spawn()

        for bullet in self.iter_all_bullets():
            if bullet.visible:
                bullet.tick()

        target_objects = [
            obj for obj in
            itertools.chain(
                self.triangles,
                self.squares,
                self.pentagons,
                [client.hero for client in self.clients],
                self.iter_all_bullets()
            )
            if obj.visible
        ]

        for obj in target_objects:
            obj.tick_pos()

        for obj, obk in bounding_box_collision_pairs(target_objects):
            collide(obj, obk)

        for client in self.clients:
            client.hero.tick_keys(self)

        self.send_updates()

    def send_updates(self):
        self.tick_id += 1
        self.broadcast_updates()
        self.broadcast_message(json.dumps({
            "tick": self.tick_id,
            "players": [
                client.hero.to_player_dict() for client in self.clients
            ],
            "squares": [square.to_dict() for square in self.squares],
            "triangles": [triangle.to_dict() for triangle in self.triangles],
            "pentagons": [pentagon.to_dict() for pentagon in self.pentagons]
        }))

    def broadcast_updates(self):
        removal = set()
        for client in self.clients:
            try:
                client.send_updates()
            except WebSocketClosedError:
                removal.add(client)
        self.clients -= removal

    def broadcast_message(self, message):
        removal = set()
        for client in self.clients:
            try:
                client.write_message(message)
            except WebSocketClosedError:
                removal.add(client)
        self.clients -= removal


arena = Arena()


class ArenaHandler(WebSocketHandler):

    def open(self):
        self.hero = Hero()
        arena.clients.add(self)
        logger.info('%s connected', self.request.remote_ip)

    def on_message(self, message):
        data = json.loads(message)
        self.hero.last_control = data
        logger.debug('%s %r', self.request.remote_ip, data)

    def send_updates(self):
        self.write_message({'self': self.hero.to_self_dict()})

    def check_origin(self, origin):
        return True

    def on_close(self):
        logger.info('%s closed', self.request.remote_ip)
