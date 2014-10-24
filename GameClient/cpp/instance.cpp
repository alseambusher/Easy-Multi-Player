#include"baseInstance.cpp"

class Instance: public BaseInstance{
	// TODO put your implementation here
	public:
	// call parent constructor
	Instance(string server_session_key): BaseInstance(server_session_key){}
};
