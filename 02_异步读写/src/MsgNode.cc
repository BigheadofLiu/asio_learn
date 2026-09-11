#include "MsgNode.hpp"

MsgNode::MsgNode(const char* msg,int total_length):_total_length(total_length),_cur_length(0)
{
    _msg=new char[total_length]; //申请内存空间
    memcpy(_msg,msg,total_length); //拷贝字符串
}

MsgNode::MsgNode(int total_length):_total_length(total_length),_cur_length(0){
    _msg=new char[total_length];
}

MsgNode::~MsgNode()
{  
    delete[] _msg;
}
