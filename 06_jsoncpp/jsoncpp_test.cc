#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <ostream>

int main(){
    Json::Value actress_1;
    actress_1["name"] = "Yu Shinoda";
    actress_1["cup"] = "D";
    std::string info =actress_1.toStyledString();  
    std::cout<<"info is:"<<info<<std::endl;

    Json::Value actress_2;
    Json::Reader reader;
    reader.parse(info,actress_2);
    std::cout<<"name:"<<actress_2["name"]<<","<<"cpu:"<<actress_2["cup"]<<std::endl;
    return 0;
}