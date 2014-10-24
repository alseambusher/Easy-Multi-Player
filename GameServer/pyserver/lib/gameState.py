from basic import Mapper
class GameState():
    def __init__(self,json_params):
        self.data = Mapper(json_params)

    def to_json(self):
        return self.data.to_dict()
