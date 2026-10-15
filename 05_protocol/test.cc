#include <iostream>
#include "student.pb.h"
#include <string>

int main(){
    student stu1;
    stu1.set_name("丁真珍珠");
    stu1.set_age(25);

    std::string data;
    stu1.SerializeToString(&data); //序列化
    std::cout<<"serialize str is:"<<data<<std::endl;  //打印序列化后的字符

    student stu2;
    stu2.ParseFromString(data);  //反序列化
    std::cout<<stu2.name()<<" "<<stu2.age()<<std::endl;

    return 0;
}
