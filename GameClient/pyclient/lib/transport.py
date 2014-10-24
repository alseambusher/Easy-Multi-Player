import zmq
import json
import traceback
import config
import time
import threading
from lib import logging
from res import actions


class Transport():
    data_push = []
    data_sub = []
    response = {}

    def __init__(self):
        self.threads = [
            threading.Thread(target=self.push_server),
            threading.Thread(target=self.alive)
        ]
        # step 1 start all servers
        for thread in self.threads:
            thread.start()

        # step 2 connect to api server
        self.send(action=actions.connect,api=True)

        # step 3 subscribe to client shared key
        self.subscribe(config.SHARED_KEY)

    # call this method to subscribe to a new key
    def subscribe(self, key):
        thread = threading.Thread(target=self.subscribe_server, args=(key,))
        thread.start()
        self.threads.append(thread)

    # call this to push to the api server
    def send(self, query={}, action=None, api=False, shared=False, session=False, server_shared=False):
        query['time'] = time.time()
        if action:
            query['action'] = action
        if api:
            query['api_key'] = config.API_KEY
        if shared:
            query['shared_key'] = config.SHARED_KEY
        if session:
            query['session_key'] = config.SESSION_KEY
        if server_shared:
            query['server_shared_key'] = config.SERVER_SHARED_KEY

        self.data_push.append(json.dumps(query))

    # get response
    def get_response(self,action):
        while True:
            for response in self.data_sub:
                if response.has_key('action') and response['action'] == action:
                    self.data_sub.remove(response)
                    return response

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

        socket.setsockopt(zmq.SUBSCRIBE, str(key))
        while True:
            try:
                data = socket.recv()
                data = " ".join(data.split(" ")[1:])
                print logging.debug(data, "[SUBSCRIBE]")
                self.data_sub.append(json.loads(data))

                # to decrease CPU usage
                time.sleep(0.001)
            except:
                traceback.print_exc()
