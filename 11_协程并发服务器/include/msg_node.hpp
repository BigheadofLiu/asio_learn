#pragma once
#include "const.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

class session;
class logic_system;
class logic_node;

class msg_node{
friend class session;
friend logic_system;
friend logic_node;
public:
    msg_node(std::size_t total_length);
    ~msg_node();
    void clear(); //清空容器
    void set_send_data(const char*msg,std::uint16_t,std::uint32_t,std::size_t); //设置
private:
    size_t _current_length;  //当前长度
    size_t _total_length; //最大长度
    std::vector<char> _data; //vector 容器 存储消息
};

class recv_msgnode:public msg_node{
    friend logic_system;
    friend logic_node;
public:
    recv_msgnode(std::size_t total_length,std::uint16_t msg_id);
private:
    std::uint16_t _msg_id;
};

class send_msgnode:public msg_node{
    friend logic_system;
    friend logic_node;
public:
    send_msgnode(const char* msg,std::size_t total_length,std::uint16_t msg_id);
private:
    std::uint16_t _msg_id;
};