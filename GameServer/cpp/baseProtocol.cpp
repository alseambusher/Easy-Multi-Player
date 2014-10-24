using namespace std;
class BaseProtocol{
	public:
	static Json::Value get_games(vector<Instance>);
	static Instance new_game(Instance,string);
	static GameState event(Json::Value);
	static GameState event(string data);
	static void dead(Json::Value);
	static void dead(string);
	static void disconnect(Json::Value);
	static void disconnect(string);

};

Json::Value BaseProtocol::get_games(vector<Instance> instances){
	Json::Value data(Json::arrayValue);
	for(vector<Instance>::iterator i=instances.begin(); i!=instances.end(); ++i)
		data.append(Json::Value((*i).session_key));
	return data;
}
Instance BaseProtocol::new_game(Instance instance,string client_shared_key){
	instance.join_instance(client_shared_key);
	return instance;
}
GameState BaseProtocol::event(Json::Value data){
	logging::debug(data.toStyledString());
	// TODO do some processing here
	return GameState();
}
GameState BaseProtocol::event(string data){
	return event(JSON::parse(data));
}

// TODO
void BaseProtocol::dead(Json::Value data){}
void BaseProtocol::dead(string data){}
void BaseProtocol::disconnect(Json::Value data){}
void BaseProtocol::disconnect(string data){}
