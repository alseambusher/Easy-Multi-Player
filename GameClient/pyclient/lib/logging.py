import config
import time


def getCurrentTime():
    return time.strftime("%d-%m-%Y %H:%M:%S")


def debug(message, key=""):
    return getCurrentTime() + ":" + key + ":" + message

# TODO add these tags
def error(message):
    errorLog = open(config.ERROR_LOG, "a")
    errorLog.write("\n" + debug(message, "[ERROR]"))
    errorLog.close()
    return debug(message, "[ERROR]")


def access(ip, type=""):
    accessLog = open(config.ACCESS_LOG, "a")
    accessLog.write("\n" + getCurrentTime() + ":" + ip + ":" + type)
    accessLog.close()


def log(args):
    logFile = open(config.LOG, "a")
    logFile.write("\n" + getCurrentTime())
    for arg in args:
        logFile.write(arg + " ")
    logFile.close()
