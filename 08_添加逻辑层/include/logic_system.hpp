#pragma once
#include "csession.hpp"
#include "logic_node.hpp"
#include "msgnode.hpp"
#include "singleton.hpp"
#include "cserver.hpp" 

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

class logic_node;
class csession;
using full_call_back = std::function<void(std::shared_ptr<csession>,std::uint16_t msg_id,const std::string msg_data)>;
class logic_system:public singleton<logic_system>{
    friend singleton<logic_system>;  //使能访问 private
public:
    ~logic_system();  //析构函数
    void post_logic_msg_to_queue(std::shared_ptr<logic_node> logic_node); //加入逻辑线程队列
private:
    logic_system(); //默认构造
    void deal_msg(); //处理数据
    void register_call_back(); //注册回调？
    void hello_world_call_back(std::shared_ptr<csession> session,std::uint16_t msg_id,const std::string msg_data); //回调
    std::thread _work_thread; //工作线程
    std::queue<std::shared_ptr<logic_node>> _logic_node_queue; //逻辑线程队列
    std::mutex _mutex; //互斥量
    std::condition_variable _consume;  //线程同步
    bool _b_stop; //优雅退出
    std::map<std::uint16_t,full_call_back> _full_call_backs;  //id <---> full_call_back 根据 id 找到对应的处理函数
    enum{
        _msg_hello_world = 1001,
        _msg_alive = 1002,
        _msg_chat = 1003
    };
};