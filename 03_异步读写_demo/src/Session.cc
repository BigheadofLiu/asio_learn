#include "Session.hpp"
#include <cstring>
#include <functional>
#include <iostream>
#include <sys/socket.h>

void Session::start(){
    //开始异步读
    memset(_data, 0, max_length);
    auto self = shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data,max_length),
    std::bind(&Session::handle_read,self,std::placeholders::_1,std::placeholders::_2));
}

Session::~Session(){
    std::cout<<"session destroy:"<<this<<"\n";
}

void Session::handle_write(const boost::system::error_code& ec){
    if(!ec){
        memset(_data, 0, max_length);
        auto self = shared_from_this();
        //echo 写法 异步
        _socket.async_read_some(boost::asio::buffer(_data,max_length),
    std::bind(&Session::handle_read,self,std::placeholders::_1,std::placeholders::_2));
    }else{
        std::cout<<"write error,error code is:"<<ec.value()<<"message is:"<<ec.message()<<"\n";
        // delete this;
    }
}

void Session::handle_read(const boost::system::error_code& ec,size_t byte_transferred){
    if(ec== boost::asio::error::eof){
        // std::cout<<"==========================\n";
        std::cout<<"ip:"<<this->_socket.remote_endpoint().address()<<" "<<"port："<<this->_socket.remote_endpoint().port()<<" ";
        std::cout<<"client close connect"<<"\n";
        // std::cout<<"==========================\n";
        return;
    }
    if(!ec){
        std::cout<<"server recived massage is:";
        std::cout.write(_data, byte_transferred);
        std::cout << std::endl;
        auto self = shared_from_this();
        
        // memset(_data, 0, max_length);
        // _socket.async_read_some(boost::asio::buffer(_data,max_length),
        // bind(&Session::handle_read,self,std::placeholders::_1,std::placeholders::_2));

        //echo 写法 异步写回数据
        boost::asio::async_write(_socket,boost::asio::buffer(_data,byte_transferred),
        std::bind(&Session::handle_write,self,std::placeholders::_1));
        // boost::asio::async_write(_socket,boost::asio::buffer("hello client"),
        // std::bind(&Session::handle_write,self,std::placeholders::_1));
    }else{
        std::cout<<"read error,error code is:"<<ec.value()<<" "<<"message is:"<<ec.message()<<"\n";
        // delete this;
    }
}
