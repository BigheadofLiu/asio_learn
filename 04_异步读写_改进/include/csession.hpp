#pragma once
#include "msgnode.hpp"
#include <boost/asio.hpp>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>


class cserver;
// #define MAX_LENGTH 1024;

class csession:public std::enable_shared_from_this<csession>{
    public:
    csession(boost::asio::io_context& ioc,cserver* cserver);
    void start();
    void send(const char* msg,size_t total_length);
    boost::asio::ip::tcp::socket& get_socket();
    const std::string& get_uuid() const;
    void print_recv_data(char* data,int length);
    private:
    boost::asio::ip::tcp::socket _socket;
    cserver* _cserver;
    std::string _uuid;
    enum{
        _max_length=1024*2,
        _head_length=sizeof(size_t)
    };
    char _data[_max_length];  //改为 vector 会不会更好？
    // std::vector<char> _data;
    void handle_read(const boost::system::error_code& ec,size_t byte_transfered);
    void handle_write( const boost::system::error_code& ec);
    std::queue<std::shared_ptr<msgnode>> _send_queue; //发送队列
    std::queue<std::shared_ptr<msgnode>> _recv_queue; //接受队列
    bool _recv_padding;   //接收状态标志  true:正在接收
    bool _send_padding;   //发送状态标志  false:未在发送
    std::mutex _send_lock; //发送锁 
    std::mutex _recv_lock; //接受锁

    //用于切包处理
    std::shared_ptr<msgnode> _recv_msg_node; //消息正文
    std::shared_ptr<msgnode> _recv_head_node; //消息头
    bool _head_status; //头节点状态 true:消息头已收完，正在收正文 false:消息头未收完
};
