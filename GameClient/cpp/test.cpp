#include <cstdio>
#include <cstring>

// This is the JSON header
#include "json/json.h"

using namespace std;

int main(int argc, char **argv)
{
 string json_example = "{\"array\": \
                            [\"item1\", \
                            \"item2\"], \
                            \"not an array\": \
                            \"asdf\" \
                         }";

 // Let's parse it  
 Json::Value root;
 Json::Reader reader;
 bool parsedSuccess = reader.parse(json_example, 
                                   root, 
                                   false);
  
 if(not parsedSuccess)
 {
   // Report failures and their locations 
   // in the document.
   cout<<"Failed to parse JSON"<<endl 
       <<reader.getFormatedErrorMessages()
       <<endl;
   return 1;
 }
 if(root["a"] == Json::Value::null)
cout<<"sdcasdcasdcs";
 // Let's extract the array contained 
 // in the root object
 const Json::Value array = root["array"];
 // Iterate over sequence elements and 
 // print its values
 for(unsigned int index=0; index<array.size(); 
     ++index)  
 {  
   cout<<"Element " 
       <<index 
       <<" in array: "
       <<array[index].asString()
       <<endl;
 }
  
 // Lets extract the not array element 
 // contained in the root object and 
 // print its value
//root["not an array2"]=array["item1"].asString();
 const Json::Value notAnArray = 
               root["not an array"];
 
 if(not notAnArray.isNull())
 {
   cout<<"Not an array: "
       <<notAnArray.asString()
       <<endl;
 }
 
 // If we want to print JSON is as easy as doing:
 cout<<"Json Example pretty print: "
     <<endl<<root.toStyledString()
     <<endl;
 
 return 0;
}
