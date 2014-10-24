import threading
import zmq
import copy
import config
import logging
import time
import json
from res import actions
import db
from basic import data_filter
from keys import session_key_gen


class Transport():
    logs = []
    data_pub = {} # this should have 'session_key':'<data_to_be_pushed>' dictionary
    connected = {} # 'shared_key':time.time()

    def __init__(self):
        self.threads = [
            threading.Thread(target=self.publish),
            threading.Thread(target=self.batch_logger),
            threading.Thread(target=self.dead_buster)
        ]

        for port in config.PORT_PULL:
            self.threads.append(threading.Thread(target=self.pull, args=(port,)))

    def start(self):
        for thread in self.threads:
            thread.start()

    def send(self, query, action, key):
        query['time'] = time.time()
        if action:
            query['action'] = action

        self.data_pub[key] = json.dumps(query)

    # keeps logging in a batch
    def batch_logger(self):
        while True:
            for log in self.logs:
                # TODO
                self.logs.remove(log)
            time.sleep(5)

    # disconnects all the clients which are not responding
    def dead_buster(self):
        while True:
            iterator = self.connected.keys()[:]
            for client_shared_key in iterator:
                if time.time() - self.connected[client_shared_key] > float(config.ALIVE_TIMEOUT):
                    try:
                        client = db.get_user_from_shared_key(client_shared_key)
                        print logging.debug("disconnect " + client.user.shared_key, "[INFO]")
                        data = {'shared_key': client.user.shared_key}
                        self.send(data, action=actions.dead, key=client.server_shared_key)
                        del self.connected[client.user.shared_key]
                    except:
                        # todo take care of servers not responding also. You have to remove all the clients if the server is not responding
                        pass
            time.sleep(5)

    def pull_processor(self, data):
        _action = data['action']

        # from game_server or client: api_key
        if _action == actions.connect:
            end_user = db.get_user_from_api_key(data['api_key'])
            self.connected[end_user.user.shared_key] = time.time()
            print logging.debug("Connect " + data['api_key'], "[Info]")

        elif _action == actions.alive:
            self.connected[data['shared_key']] = float(data['time'])

        # from client api_key,
        elif _action == actions.get_games:
            # TODO if client doesnt exist then raise error
            client = db.get_user_from_api_key(data['api_key'])
            print logging.debug(actions.get_games, '[Info]')
            self.send({'shared_key': client.user.shared_key, }, action=actions.get_games, key=client.server_shared_key)

        # from game server
        elif _action == actions.game_list:
            # replace server shared key with client shared key
            data['shared_key'] = data['client_shared_key']
            key = data['client_shared_key']
            # remove that field
            data = data_filter(data, ['client_shared_key', ])
            # publish to that key
            self.send(data, action=actions.game_list, key=key)

        # from client select game shared_key
        elif _action == actions.select_game:
            client = db.get_user_from_shared_key(data['shared_key'])
            self.send(data, action=actions.select_game, key=client.server_shared_key)


        # from client new game: shared_key
        elif _action == actions.new_game:
            client = db.get_user_from_shared_key(data['shared_key'])
            data['session_key'] = session_key_gen()
            # send server
            self.send(data, action=actions.new_game, key=client.server_shared_key)
            data = data_filter(data, ['shared_key', ])
            # send to client
            self.send(data, action=actions.game_session, key=client.user.shared_key)

        # from client, to make things faster server_shared_key is also sent
        elif _action == actions.event:
            key = data['server_shared_key']
            data = data_filter(data, ['server_shared_key'])
            self.send(data, action=actions.event, key=key)

        # from server. forward this to all clients
        elif _action == actions.game_state:
            session_key = data['session_key']
            data = data_filter(data, ['session_key'])
            self.send(data, action=actions.game_state, key=session_key)

    def pull(self, port):
        context = zmq.Context()
        socket = context.socket(zmq.PULL)

        # listening to client
        socket.bind("tcp://*:" + port)

        while True:
            string = socket.recv()
            data = json.loads(string)
            if data['action'] != actions.alive:
                print logging.debug(string, '[PULL:' + port + ']')
            thread = threading.Thread(target=self.pull_processor, args=(data,))
            thread.start()

            time.sleep(0.001)

    def publish(self):
        context = zmq.Context()
        socket = context.socket(zmq.PUB)

        socket.bind("tcp://*:" + config.PORT_PUB)
        while True:
            try:
                data_queue = copy.deepcopy(self.data_pub)
            except RuntimeError:
                pass
            for session_key in data_queue:
                socket.send(str(session_key + " " + data_queue[session_key]))
                del (self.data_pub[session_key])

            time.sleep(0.001)
