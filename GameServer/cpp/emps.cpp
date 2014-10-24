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
void run(){
	// TODO put the stuffs here
	cout<< "nothing to run";
}
int main(int argv, char** argc){
	//logging::debug("a","b");
	//BaseInstance bi = BaseInstance(SESSION_KEY);
	//bi.join_instance("alse");
	//bi.leave_instance("alse");
	//cout<<bi.status;
	/*cout<<JSON::parse("{\"array\": \
                            [\"item1\", \
                            \"item2\"], \
                            \"not an array\": \
                            \"asdf\" \
                         }")["not an array"].asString();*/
	Transport server = Transport(run);

	// this should always be in the end. This just has thread join :P
	return 0;
}
