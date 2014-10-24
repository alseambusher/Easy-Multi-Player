#include<ctime>
#include<fstream>

class logging{
	public:
	static char* getCurrentTime(char* result){
		time_t t = time(0);
		struct tm *now = localtime(&t);
		sprintf(result,"%d-%d-%d %d:%d:%d",now->tm_mday, now->tm_mon, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);
		return result;
	}
	
	static void debug(string message, string key=""){
		char *time=(char*)malloc(20);

		ofstream log;
		log.open("log",ios::app);
		log<<getCurrentTime(time)<< key<< message<<"\n";
		log.close();
		
		cout<<getCurrentTime(time)<< key<< message;
		free(time);
	}
};
