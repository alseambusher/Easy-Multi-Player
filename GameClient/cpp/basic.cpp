#include <cstdio>
#include <cstring>
using namespace std;
class JSON{
	public:
	static Json::Value parse(string json_string){
		Json::Value root;
 		Json::Reader reader;
 		bool parsedSuccess = reader.parse(json_string, root, false);
 		if(not parsedSuccess){
   			cout<<"Failed to parse JSON"<<endl 
       			<<reader.getFormatedErrorMessages()
       			<<endl;
 		}
		return root;
	}
};

