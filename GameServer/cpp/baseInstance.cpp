
using namespace std;

class BaseInstance{
	public:
	vector<string> users;
	string status;
	string session_key;
	GameState game_state;

	BaseInstance(string);
	void join_instance(string);
	void leave_instance(string);
};

BaseInstance::BaseInstance(string server_session_key){
	session_key = server_session_key;
	game_state = GameState();
}

void BaseInstance::join_instance(string client_shared_key){
	status = "";
	users.push_back(client_shared_key);
}
void BaseInstance::leave_instance(string client_shared_key){
	for(vector<string>::iterator search = users.begin(); search != users.end(); ++search){
		if(*search == client_shared_key){
			users.erase(search);	
			break;
		}
	}
	if(users.begin() == users.end())
		status = "removed";
}
