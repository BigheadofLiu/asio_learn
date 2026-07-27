#pragma once
#include <iostream>
#include <memory>
#include <cstring> //using memcpy
const int RECVSIZE {1024};

//消息节点类 用于管理发送或接收的数据
class MsgNode
{
public:
    int _total_length;  //消息字符总长度
    int _cur_length;  //已处理字符长度（已读或已写）
    char* _msg; //消息字符首地址 用于拷贝传入的字符
public:
    MsgNode(const char* msg,int total_length);
    MsgNode(int total_length);
    ~MsgNode();
};

