class GameState{
	public:
	Json::Value data;
	GameState(){
		data["gold"] = 1000;
		data["height"] = 2000;
		data["x"] = 20;
	}
};
