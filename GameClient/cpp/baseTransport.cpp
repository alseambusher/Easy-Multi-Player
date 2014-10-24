#include<time.h>
#include<sstream>
#define len(x) (sizeof(x)/sizeof(x[0]))

using namespace std;
// use usleep instead of sleep
class BaseTransport{
	public :
	vector<pthread_t> threads;
	vector<string> data_push;
	vector<Json::Value> data_sub;
	BaseTransport(void(*run)()){
		send(Json::Value::null,ACTION_CONNECT,true,false,false,false);

		thread push_server_thread(push_server,this);
		thread alive_thread(alive,this);
		thread run_thread(run);
		subscribe(SHARED_KEY);

		alive_thread.join();
		push_server_thread.join();
		run_thread.join();
	}
	void subscribe(string);
	void send(Json::Value, string, bool, bool,bool,bool);
	static void parse_response(BaseTransport*,Json::Value);
	static void alive(BaseTransport*);
	static void push_server(BaseTransport*);
	static void subscribe_server(BaseTransport*,string);
	Json::Value get_response(string);
};

void BaseTransport::subscribe(string key){
	thread subscribe_server_thread(subscribe_server,this,key);
	// You have to detach because the function subscribe(...) gets terminated here
	subscribe_server_thread.detach();
}
void BaseTransport::send(Json::Value query, string action="", bool api=false, bool shared=false, bool session=false ,bool server_shared=false){
	time_t t = time(0);
	query["time"]= (int) t;

	if(action!="")
		query["action"] = action;
	if(api)
		query["api_key"] = API_KEY; 
	if(shared)
		query["shared_key"] = SHARED_KEY;
	if(session)
		query["session_key"] = SESSION_KEY;
	if(server_shared)
		query["server_shared_key"] = SERVER_SHARED_KEY;

	logging::debug(query.toStyledString());	
	data_push.push_back(query.toStyledString());
}
void BaseTransport::parse_response(BaseTransport* self, Json::Value data){
	// this has to be overrided in child class
}
Json::Value BaseTransport::get_response(string action){
	while(true){
		for(int i=0; i<data_sub.size(); i++){
			if((data_sub[i]["action"] != Json::Value::null) && (data_sub[i]["action"].asString() == action)){
				Json::Value response = data_sub[i];
				data_sub.erase(data_sub.begin()+i+1);
				return response;
			}
				
		}
	}
}

int i=0;
void BaseTransport::alive(BaseTransport* T){
	while(true){
		T->send(Json::Value::null,ACTION_ALIVE,false,true);
		chrono::milliseconds dura(1000*ALIVE_PULSE);
		this_thread::sleep_for(dura);
	}
}
void BaseTransport::push_server(BaseTransport* self){
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
void BaseTransport::subscribe_server(BaseTransport* self, string key){
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
		self->data_sub.push_back(JSON::parse(data.c_str()));
		thread parse_response_thread(parse_response,self,JSON::parse(data.c_str()));
		parse_response_thread.detach();
		usleep(10);
	}
}
