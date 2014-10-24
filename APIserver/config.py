#!/bin/env python
# ENV VARS
HOST = "127.0.0.1"
PORT_PUB = "6001"
PORT_PULL = ["6002", "6003", "6004", "6005", "6006"]
RES = "res"
DB = RES + "/data.db"

#FILES
#logs
ERROR_LOG = RES + "/error.log"
ACCESS_LOG = RES + "/access.log"
LOG = RES + "/log.log"

ALIVE_TIMEOUT=120
