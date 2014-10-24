
# converts json to object
class Mapper(object):
    def __init__(self, d):
        self.__dict__['data'] = d

    def __getattr__(self, key):
        value = self.__dict__['data'][key]
        if type(value) == type({}):
            return Mapper(value)

        return value

    def to_dict(self):
        return self.__dict__['data']