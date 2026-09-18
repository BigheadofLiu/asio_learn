#include "msgnode.hpp"
#include <cstring>

msgnode::msgnode(const char* msg,size_t total_len){
    _data=new char[total_len];
    memcpy(_data, msg, total_len);
}
msgnode::~msgnode(){
    delete []_data;
}

