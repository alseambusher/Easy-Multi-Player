import sqlalchemy
import config
from models import *
from sqlalchemy.orm import sessionmaker
import json

#engine = sqlalchemy.create_engine("sqlite:///" + config.DB, echo=False)
engine = sqlalchemy.create_engine(
                "mysql://root:alse@localhost"
            )
# todo move this to config
db_name="test"
def new_session():
    engine.execute("USE "+db_name) # select new db
    Session = sessionmaker(bind=engine)
    return Session()


def setup():
    Base.metadata.create_all(engine)

def new_game_server(user_json):
    session = new_session()
    user = User(json.dumps(user_json))
    server = GameServer()
    server.user = user
    session.add(server)
    session.commit()
    return server


def new_game_client(user_json, server_shared_key):
    session = new_session()
    user = User(json.dumps(user_json))
    client = GameClient(server_shared_key)
    client.user = user
    session.add(client)
    session.commit()
    return client

def get_user_from_api_key(api_key):
    session = new_session()
    user = session.query(User).filter(User.api_key == api_key).first()
    game_client = session.query(GameClient).filter(GameClient.user == user).first()
    if game_client:
        return game_client
    game_server = session.query(GameServer).filter(GameServer.user == user).first()
    if game_server:
        return game_server
    return None

def get_user_from_shared_key(shared_key):
    session = new_session()
    user = session.query(User).filter(User.shared_key == shared_key).first()
    game_client = session.query(GameClient).filter(GameClient.user == user).first()
    if game_client:
        return game_client
    game_server = session.query(GameServer.filter(GameServer.user == user)).first()
    if game_server:
        return game_server
    return None