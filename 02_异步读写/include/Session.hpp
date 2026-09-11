#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <iostream>
#include <queue>
#include "MsgNode.hpp"

namespace asio = boost::asio;

class Session{
    public:
    using asio_socket=boost::asio::ip::tcp::socket;
    using asio_endpoint=boost::asio::ip::tcp::endpoint;
    using error_code=boost::system::error_code;

    Session(std::shared_ptr<asio_socket> socket);
    void Connect(asio_endpoint& ep);

    //异步写相关函数
    //写操作 error
    // void WriteCallBackErr(boost::system::error_code& ec,
    //     std::size_t bytetransferred,std::shared_ptr<MsgNode> msg_node);
    void WriteCallBack(const boost::system::error_code& ec,  //需要多次回调 使用 socket.write_some()
        std::size_t bytetransferred);
    void WriteALLCallBack(const boost::system::error_code& ec,  //使用 asio::write() 只需回调一次
        std::size_t bytetransferred);
    
    //写操作的回调函数 error
    // void WriteToSocketErr(const std::string& buf);
    void WriteToSocket(const std::string& buf); //需要多次回调 socket.write_some()
    void WriteALlToSocket(const std::string& buf); //使用 asio::write() 只需回调一次

    //异步读相关函数 使用 async_read_some() 或 asio::async_receive()
    void ReadFromSocket();
    void ReadALlFromSocket();

    void ReadCallBack(const boost::system::error_code& ec,
        std::size_t bytetransferred);
    void ReadAllCallBack(const boost::system::error_code& ec,
        std::size_t bytetransferred);
    
    private:
    std::shared_ptr<asio_socket> _socket;
    // std::shared_ptr<MsgNode> _send_node; //发送节点 error 这里是错误示范 不能保证字符的前后顺序 正确的方法是使用队列
    std::queue<std::shared_ptr<MsgNode>> _send_queue; //写消息队列

    // std::queue<std::shared_ptr<MsgNode>> _recv_queue; //读消息队列 error 不考虑粘包的情况读消息不需要队列
    std::shared_ptr<MsgNode> _recv_node;

    bool _send_pending; //节点数据发送状态 true 未发送完 flase 发送完毕  是否有节点正在发送
    bool _recv_pending; //是否有数据正在接收
};