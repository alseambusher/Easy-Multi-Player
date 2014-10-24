#include<pthread.h>
#include<time.h>
#include<sstream>
#define len(x) (sizeof(x)/sizeof(x[0]))

using namespace std;
// use usleep instead of sleep
class Transport{
	public :
	vector<pthread_t> threads;
	vector<string> data_push, data_sub;
	vector<Instance> instances;
	
	Transport(void(*run)()){
		thread push_server_thread(push_server,this);
		thread alive_thread(alive,this);
		thread subscribe_server_thread(subscribe_server,this,SHARED_KEY);
		thread run_thread(run);

		send(Json::Value::null,ACTION_CONNECT,true,false);

		alive_thread.join();
		push_server_thread.join();
		subscribe_server_thread.join();
		run_thread.join();
	}
	void subscribe(string);
	void send(Json::Value, string, bool, bool);
	static void parse_response(Transport*,Json::Value);
	Instance* get_instance(string);
	static void alive(Transport*);
	static void push_server(Transport*);
	static void subscribe_server(Transport*,string);
};

void Transport::subscribe(string key){
	thread subscribe_server_thread(subscribe_server,this,key);
	subscribe_server_thread.join();
}
void Transport::send(Json::Value query, string action="", bool api=false, bool shared=false){
	time_t t = time(0);
	query["time"]= (int) t;

	if(action!="")
		query["action"] = action;
	if(api)
		query["api_key"] = API_KEY; 
	if(shared)
		query["shared_key"] = SHARED_KEY;
	logging::debug(query.toStyledString());	
	data_push.push_back(query.toStyledString());
}
void Transport::parse_response(Transport* self, Json::Value data){
	string action = data["action"].asString();
	if (action == ACTION_GET_GAMES){
		data["client_shared_key"] = data["shared_key"];
		data["games"] = Protocol::get_games(self->instances);
		self->send(data, ACTION_GAME_LIST, false, true);
	}
	else if (action == ACTION_NEW_GAME){
		Instance new_instance = Instance(data["session_key"].asString());
		new_instance = Protocol::new_game(new_instance,data["shared_key"].asString());
		self->instances.push_back(new_instance);
	}
	else if(action == ACTION_SELECT_GAME){
		Instance* instance = self->get_instance(data["session_key"].asString());
		instance->join_instance(data["shared_key"].asString());
	}
	else if(action == ACTION_EVENT){
		Instance* instance = self->get_instance(data["session_key"].asString());
		instance->game_state = Protocol::event(data);
		Json::Value data = instance->game_state.data;
		data["session_key"] = instance->session_key;
		self->send(data,ACTION_GAME_STATE,false,true);
	}
	else if(action == ACTION_DEAD){
		Instance* instance = self->get_instance(data["session_key"].asString());
		instance->leave_instance(data["shared_key"].asString());
		Protocol::dead(data);
		if(instance->status == "removed")
			free(instance);
	}
	else if(action == ACTION_DISCONNECT){
		Instance* instance = self->get_instance(data["session_key"].asString());
		instance->leave_instance(data["shared_key"].asString());
		Protocol::disconnect(data);
		if(instance->status == "removed")
			free(instance);
	}
}
Instance* Transport::get_instance(string session_key){
	for(vector<Instance>::iterator it=instances.begin(); it!=instances.end(); ++it){
		if((*it).session_key==session_key)
			return &(*it);
	}
}
int i=0;
void Transport::alive(Transport* T){
	while(true){
		T->send(Json::Value::null,ACTION_ALIVE,false,true);
		chrono::milliseconds dura(1000*ALIVE_PULSE);
		this_thread::sleep_for(dura);
	}
}
void Transport::push_server(Transport* self){
	zmq::context_t context (1);
    	zmq::socket_t socket (context, ZMQ_PUSH);
	for(int i=0;i<len(PORT_PUSH);i++){
		string addr = "tcp://" + HOST + ":" + PORT_PUSH[i]; 
    		socket.connect(addr.c_str());

	}
	while(true){
		vector<string> data_copy = self->data_push;
		self->data_push.clear();
		for(int i=0;i<data_copy.size();i++){
			char *data = (char*)(data_copy[i].c_str());
			zmq::message_t push_message(strlen(data))	;
			memcpy ((void *) push_message.data(), data, strlen(data));
        		socket.send (push_message);
		}
		usleep(10);
	}
}
void Transport::subscribe_server(Transport* self, string key){
	cout<<key;
	zmq::context_t context (1);
    	zmq::socket_t socket (context, ZMQ_SUB);
	string addr = "tcp://" + HOST + ":" + PORT_SUB; 
	socket.connect(addr.c_str());
	socket.setsockopt(ZMQ_SUBSCRIBE,key.c_str(),strlen(key.c_str()));
	while(true){
		zmq::message_t reply_data;
		socket.recv(&reply_data);
		stringstream ss;
		string data;
		ss << (char*)reply_data.data();
		data = ss.str();
		data = data.substr(data.find("{"));
		data = data.substr(0,data.find_last_of("}")+1);
		
		logging::debug(data.c_str(), "[SUBSCRIBE]");
		thread parse_response_thread(parse_response,self,JSON::parse(data.c_str()));
		parse_response_thread.join();
		usleep(10);
	}
}
