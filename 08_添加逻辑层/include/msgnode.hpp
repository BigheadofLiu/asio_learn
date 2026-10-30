#pragma once
#include "logic_system.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

class csession;
class logic_system;

class msgnode{
public:
    friend csession;
    //原构造函数用于实现发送和接收数据的节点，现改为对应的类
    // msgnode(const char* msg,size_t total_len); 
    // msgnode(size_t total_len);
    msgnode(std::size_t);
    void clear();
protected:
    enum{
        _max_size=1024*2, //消息正文最大长度
        _head_length=sizeof(uint32_t), //消息头长度
        _id_length=sizeof(uint16_t),  //id 长度
        _head_total_length=_head_length+_id_length  //id+消息头部
    };
    size_t _cur_length;
    size_t _max_length;
    std::vector<char> _data;  //裸指针改为 vector 避免手动释放
};

class recv_msgnode:public msgnode{
    friend csession;
    friend logic_system;
public: 
    recv_msgnode(size_t total_length,uint16_t msg_id);
private:
    uint16_t _msg_id;
};

class send_msgnode:public msgnode{
    friend csession;
public:
    send_msgnode(const char* msg,size_t total_length,uint16_t msg_id);
private:
    uint16_t _msg_id;
};