#include"baseTransport.cpp"

class Transport: public BaseTransport{
	public:
	Transport(void(*run)()) : BaseTransport(run){
	}
	static void parse_response(Transport*, Json::Value);
};

void Transport::parse_response(Transport* self, Json::Value data){
	// user defined
	if(data["action"] != Json::Value::null){
		string action = data["action"].asString();
	}
}
