#include "logic_system.hpp"
#include "csession.hpp"
#include "logic_node.hpp"
#include "msgnode.hpp"
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <ostream>
#include <thread>

logic_system::logic_system():
    _b_stop(false)
{
    register_call_back();
    _work_thread=std::thread(&logic_system::deal_msg,this);
}

void logic_system::register_call_back(){
    //匿名函数
    _full_call_backs[_msg_hello_world]=[this](auto arg_1,auto arg_2,auto arg_3){
        hello_world_call_back(arg_1,arg_2,arg_3);
    };

    //bind
    // _full_call_backs[_msg_hello_world]=std::bind(
    //     &logic_system::hello_world_call_back,
    //     this,
    //     std::placeholders::_1,
    //     std::placeholders::_2,
    //     std::placeholders::_3
    // );
}

void logic_system::hello_world_call_back(std::shared_ptr<csession> session,std::uint16_t msg_id,const std::string msg_data){
    Json::Reader read;
    Json::Value book;
    read.parse(msg_data,book);
    std::cout<<"recived data is:"<<book["data"].asString()<<" "<<"id is:"<<book["id"].asInt()<<std::endl; 
    std::string return_msg=book.toStyledString();
    session->send(return_msg,book["id"].asUInt());
}

void logic_system::deal_msg(){
    for(;;){
        std::unique_lock<std::mutex> lock(_mutex);
        while(_logic_node_queue.empty() && !_b_stop){
            _consume.wait(lock);
        }
        if(_b_stop){
            auto msg_node=_logic_node_queue.front();
            std::cout<<"msg id is:"<<msg_node->_recv_node->_msg_id<<std::endl;
        }

    }
}