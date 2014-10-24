// this will have all the constants
using namespace std;
string HOST = "127.0.0.1";
string PORT_SUB = "6001";
string PORT_PUSH[5] = {"6002", "6003", "6004", "6005", "6006"};

string API_KEY = "a191e1d7a51d4ece08bb38e0b4e0ffa8252c2343";
string SHARED_KEY="87a03209ac9a198c71e32d7a70b4b442";


//string API_KEY = "e89ebdc6bf313f740a2761dca98f9c3a26a5b8e4";
//string SHARED_KEY="85c6cfb69646ed31ba97be3876327546";
string SERVER_SHARED_KEY = "b8b15ab61f3fe23b968bf72762ba3d77";

// this will be set later	
string SESSION_KEY;


// Users shouldnt change these
int ALIVE_PULSE=60;

// actions
string ACTION_CONNECT="connect";
string ACTION_GET_GAMES="get_games";
string ACTION_GAME_LIST="game_list";
string ACTION_SELECT_GAME="select_game";
string ACTION_GAME_SESSION="game_session";
string ACTION_NEW_GAME="new_game";
string ACTION_NEW_GAME_DATA="new_game_data";
string ACTION_SUBSCRIBE_GAME_STATE="subscribe_game_state";
string ACTION_EVENT="event";
string ACTION_GAME_STATE="game_state";
string ACTION_DISCONNECT="disconnect";
string ACTION_DEAD="dead";

string ACTION_ALIVE="alive";


