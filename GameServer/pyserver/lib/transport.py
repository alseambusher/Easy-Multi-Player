import zmq
import json
import traceback
import time
import threading
import logging
from userOverrides import Instance,Protocol
from res import actions
import config

class Transport():
    data_push = []
    data_sub = []
    #response = {}
    # this will have all the game instances
    instances = []

    def __init__(self):
        self.threads = [
            threading.Thread(target=self.push_server),
            threading.Thread(target=self.alive)
        ]
        # step 1 start all servers
        for thread in self.threads:
            thread.start()

        # step 2 connect to api server
        self.send(action=actions.connect, api=True)

        # step 3 subscribe to client shared key
        self.subscribe(config.SHARED_KEY)

    # call this method to subscribe to a new key
    def subscribe(self, key):
        thread = threading.Thread(target=self.subscribe_server, args=(key,))
        thread.start()
        self.threads.append(thread)

    # call this to push to the api server
    def send(self, query={}, action=None, api=False, shared=False):
        query['time'] = time.time()
        if action:
            query['action'] = action
        if api:
            query['api_key'] = config.API_KEY
        if shared:
            query['shared_key'] = config.SHARED_KEY

        self.data_push.append(json.dumps(query))


    def parse_response(self, data):
        _action = data['action']
        if _action == actions.get_games:
            data['client_shared_key'] = data['shared_key']
            data['games'] = Protocol.get_games(self, data['client_shared_key'])
            self.send(data, action=actions.game_list, shared=True)

        if _action == actions.new_game:
            new_instance=Instance(data['session_key'])
            new_instance = Protocol.new_game(new_instance,data['shared_key'])
            self.instances.append(new_instance)

        if _action == actions.select_game:
            instance = self.get_instance(data['session_key'])
            instance.join_instance(data['shared_key'])

        if _action == actions.event:
            instance = self.get_instance(data['session_key'])
            # set game state to processed game state
            # todo make game_state modification lock and process type
            instance.game_state = Protocol.event(data)
            data=instance.game_state.to_json()
            data['session_key']=instance.session_key
            self.send(data,action=actions.game_state,shared=True)

        if _action == actions.dead:
            instance = self.get_instance(data['session_key'])
            instance.leave_instance(data['shared_key'])
            Protocol.dead(data)
            if instance.status == 'removed':
                del instance

        if _action == actions.disconnect:
            instance = self.get_instance(data['session_key'])
            instance.leave_instance(data['shared_key'])
            Protocol.disconnect(data)
            if instance.status == 'removed':
                del instance

    def get_instance(self,session_key):
        for instance in self.instances:
            if str(instance.session_key) == str(session_key):
                return instance

    # this keeps sending packet to server to tell that it is alive
    def alive(self):
        while True:
            data={}
            self.send(data,action=actions.alive, shared=True)
            time.sleep(config.ALIVE_PULSE)

    def push_server(self):
        context = zmq.Context()
        socket = context.socket(zmq.PUSH)

        for port in config.PORT_PUSH:
            socket.connect("tcp://" + config.HOST + ":" + port)

        while True:
            try:
                for data in self.data_push:
                    print logging.debug(data, "[PUSH]")
                    socket.send(data)
                    self.data_push.remove(data)

                # to decrease CPU usage
                time.sleep(0.001)
            except:
                traceback.print_exc()


    # DONT start this right in the beginning coz u dont know what is the session key of the game instance
    # todo see whether client has to subscribe to many session keys
    def subscribe_server(self, key):
        context = zmq.Context()
        socket = context.socket(zmq.SUB)
        socket.connect("tcp://" + config.HOST + ":" + config.PORT_SUB)

        socket.setsockopt(zmq.SUBSCRIBE, key)
        while True:
            try:
                data = socket.recv()
                data = " ".join(data.split(" ")[1:])
                print logging.debug(data, "[SUBSCRIBE]")
                #self.data_sub.append(json.loads(data))
                thread = threading.Thread(target=self.parse_response, args=(json.loads(data),))
                thread.start()

                # to decrease CPU usage
                time.sleep(0.001)
            except:
                traceback.print_exc()