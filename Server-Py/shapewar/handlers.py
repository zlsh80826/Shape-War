import logging
import tornado.web
from tornado.websocket import WebSocketHandler, WebSocketClosedError
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm.exc import NoResultFound
from .database import DBHandlerMixin, User


logger = logging.getLogger(__name__)


class RequestHandler(DBHandlerMixin, tornado.web.RequestHandler):

    # user related stuff

    def get_current_user(self, user_cookie=None):
        if user_cookie is None:
            user_cookie = self.get_secure_cookie('user')
        if user_cookie is None:
            return None
        else:
            with self.get_session() as session:
                try:
                    return session.query(User)\
                        .filter(User.username == user_cookie).one()
                except NoResultFound:
                    return None

    def set_current_user(self, user):
        self.set_secure_cookie('user', user)

    def logout_user(self):
        self.clear_cookie('user')


class APIHandler(RequestHandler):

    def write_error(self, status_code, exc_info=None):
        self.finish({
            'error': {
                'message': 'HTTP {}: {}'.format(status_code, self._reason)
            }
        })


class APIPing(APIHandler):

    def get(self):
        self.finish({'message': 'server is running'})


class APILogin(APIHandler):

    def post(self):
        username = self.get_body_argument('username')
        password = self.get_body_argument('password')
        with self.get_session() as session:
            try:
                user = session.query(User) \
                    .filter(User.username == username).one()
            except NoResultFound:
                self.finish(
                    {'error': {'message': 'no such user'}}
                )
            else:
                if user.verify_password(password):
                    self.finish({'token': "NotImplemented"})
                else:
                    self.finish({
                        'error': {'message': 'invalid passwored'}
                    })


class APIRegister(APIHandler):

    def post(self):
        username = self.get_body_argument('username')
        password = self.get_body_argument('password')
        user = User(username=username, password=password)
        try:
            with self.get_session() as session:
                session.add(user)
        except IntegrityError:
            self.finish({'error': {'message': 'username is used already'}})
        else:
            self.set_current_user(username)
            self.finish({'token': "NotImplemented"})


class Arena:

    def __init__(self):
        self.clients = set()
        self.tick_id = 0

    def send_updates(self):
        self.tick_id += 1
        self.broadcast_message({
            "tick": self.tick_id,
            "self": {
                "maxHp": 10000,
                "currentHp": 5000,
                "experience": 400,
                "level": 10,
                "passive": {
                    "HealthRegen": 1,
                    "maxHealth": 2,
                    "bodyDamage": 1,
                    "bulletSpeed": 3,
                    "bulletPenetration": 2,
                    "bulletDamage": 1,
                    "reload": 4,
                    "movementSpeed": 3
                },
                "angle": 245,
                "x": 300 + (self.tick_id % 40),
                "y": 400
            },
            "bullets": [
                {
                    "id": 1,
                    "x": 250,
                    "y": 260 + (self.tick_id % 50)
                },
            ],
        })

    def broadcast_message(self, message):
        removal = set()
        for client in self.clients:
            try:
                client.write_message(message)
            except WebSocketClosedError:
                removal.add(client)
        self.clients -= removal


arena = Arena()


class DummyArenaHandler(WebSocketHandler):

    def open(self):
        arena.clients.add(self)
        logger.info('%s connected', self.request.remote_ip)

    def on_message(self, message):
        logger.info('%s said %r', self.request.remote_ip, message)

    def check_origin(self, origin):
        return True

    def on_close(self):
        logger.info('%s closed', self.request.remote_ip)


all_handlers = [
    ('/api/ping', APIPing),
    ('/api/login', APILogin),
    ('/api/register', APIRegister),
    ('/arena/dummy', DummyArenaHandler),
]
