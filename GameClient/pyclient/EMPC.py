from lib.transport import Transport
from lib import protocol
from res import actions
import time
import config

client = Transport()
protocol.init(client)

# todo remove this
data={'x':10,'y':20,'gold':300}
while True:
    client.send(data,action=actions.event,shared=True,session=True,server_shared=True)
    time.sleep(1.0/config.RATE)
