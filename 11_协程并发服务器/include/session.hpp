#pragma once
#include "msg_node.hpp"
#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include "const.hpp"

class server;
class session:public std::enable_shared_from_this<session>{
    public:
    session(boost::asio::io_context& ioc,server* server);
    void start();
    void send(const char* msg,size_t total_length,uint16_t msg_id);
    void send(const std::string msg,uint16_t msg_id); //重载
    boost::asio::ip::tcp::socket& get_socket();
    const std::string& get_uuid() const;
    void print_recv_data(char* data,int length);
    private:
    boost::asio::ip::tcp::socket _socket;
    server* _server;
    std::string _uuid;
    boost::asio::awaitable<void> read_loop(std::shared_ptr<session> self);  //使用协程一次性读完id+长度信息+正文
    void handle_read_head_total(const boost::system::error_code& ec,size_t byte_transfered);  //读 id 和消息长度
    void handle_read_msg(const boost::system::error_code& ec,size_t byte_transfered); //读消息正文
    void handle_write( const boost::system::error_code& ec);
    std::queue<std::shared_ptr<send_msgnode>> _send_queue; //发送队列
    std::queue<std::shared_ptr<recv_msgnode>> _recv_queue; //接受队列
    bool _recv_padding;   //接收状态标志  true:正在接收
    bool _send_padding;   //发送状态标志  false:未在发送
    std::mutex _send_lock; //发送锁 
    std::mutex _recv_lock; //接受锁
    boost::asio::strand<boost::asio::io_context::executor_type> _strand; //用于保证线程安全
    //用于切包处理
    std::shared_ptr<recv_msgnode> _recv_msg_node; //消息正文
    std::shared_ptr<msg_node> _recv_head_node; //消息头  加入消息 id
    bool _head_status; //头节点状态 true:消息头已收完，正在收正文 false:消息头未收完
};
