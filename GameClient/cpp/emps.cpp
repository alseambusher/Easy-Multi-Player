#include<stdio.h>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<zmq.hpp>
#include<vector>
#include<chrono>
#include<thread>
// This is the JSON header
#include "json/json.h"
#include "config.h"
#include "basic.cpp"
#include "logging.cpp"
#include "gameState.cpp"
#include "instance.cpp"
#include "protocol.cpp"
#include "transport.cpp"

using namespace std;
void run();
Transport server = Transport(run);
void run(){
	server.send(Json::Value::null,ACTION_GET_GAMES,true);
	Json::Value games = server.get_response(ACTION_GAME_LIST);
	cout << games.toStyledString();
	cout << "enter session_key: ";
	string key;
	cin >> key;
	if(key == "-1")
		server.send(Json::Value::null,ACTION_NEW_GAME,false,true);
	else{
		SESSION_KEY = key;
		Json::Value select_query;
		select_query["game"] = key;
		server.send(select_query,ACTION_SELECT_GAME,false,true,true);
	}
	string session_key = server.get_response(ACTION_GAME_SESSION)["session_key"].asString();
	SESSION_KEY = session_key;
	server.subscribe(SESSION_KEY);
	// TODO put the stuffs here
	Json::Value data;
	data["x"] = 10;
	data["y"] = 20;
	data["gold"] = 200;
	server.send(data,ACTION_EVENT,false,true,true,true);
}
int main(int argv, char** argc){
	return 0;
}
