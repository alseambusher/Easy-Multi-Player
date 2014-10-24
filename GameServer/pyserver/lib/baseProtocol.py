import json
from gameState import GameState
import logging
import config
class BaseProtocol():

    def __init__(self):
        pass
    @staticmethod
    def get_games(transport, client_shared_key):
        # by default it will send all games
        # do something with client_shared_key
        data=[]
        for instance in transport.instances:
            data.append(instance.session_key)

        return data

    @staticmethod
    def new_game(new_instance,client_shared_key):
        new_instance.join_instance(client_shared_key)
        return new_instance

    @staticmethod
    def event(data):
        print logging.debug(json.dumps(data))
        # by default it will return the default game state
        # do some processing here
        return GameState(config.GAME_STATE_PARAMS)

    @staticmethod
    def dead(data):
        pass

    @staticmethod
    def disconnect(data):
        pass