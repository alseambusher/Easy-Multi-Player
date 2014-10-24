import config
from gameState import GameState


class BaseInstance():
    # array of user shared keys
    users = []
    status = ''

    def __init__(self, session_key, game_state_params=config.GAME_STATE_PARAMS):
        self.session_key = session_key
        self.game_state = GameState(game_state_params)

    # add user to instance
    def join_instance(self, client_shared_key):
        self.users.append(client_shared_key)

    def leave_instance(self, client_shared_key):
        self.users.remove(client_shared_key)
        # todo test this
        # todo remove all the removed instances
        if len(self.users) == 0:
            self.status = 'removed'
