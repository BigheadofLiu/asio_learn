#include "csession.hpp"
#include "cserver.hpp"
#include "msgnode.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

csession::csession(boost::asio::io_context& ioc,cserver* cserver):_socket(ioc),_cserver(cserver),_recv_padding(false),_send_padding(false){
    auto uuid=boost::uuids::random_generator()();
    _uuid=boost::uuids::to_string(uuid);
}

void csession::start(){
    memset(_data,0,_max_legth);
   auto self=shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data,_max_legth),
    std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
}

 void csession::send(char* msg,size_t total_length){
    std::lock_guard<std::mutex> send_locker(_send_lock);
    _send_queue.push(std::make_shared<msgnode>(msg,total_length));
    if(_send_padding){
        return;
    }
    _send_padding=true;
    auto& first_msgnode=_send_queue.front();
    auto self=shared_from_this();
    boost::asio::async_write(_socket,boost::asio::buffer(first_msgnode->_data,first_msgnode->_max_length),
    std::bind(&csession::handle_write,self,std::placeholders::_1));
 }

boost::asio::ip::tcp::socket& csession::get_socket(){
    return _socket;
}

const std::string& csession::get_uuid(){
    return _uuid;
}

void csession::handle_read(const boost::system::error_code& ec,size_t buffertransfered){
    if(ec== boost::asio::error::eof){
        std::cout<<"ip:"<<_socket.remote_endpoint().address()<<" "<<"port："<<_socket.remote_endpoint().port()<<" ";
        std::cout<<"client close connect"<<"\n";
        _cserver->clear_csession(_uuid);
        return;
    }
    if(!ec){
        std::cout<<"server recived massage is:";
        std::cout.write(_data, buffertransfered);
        std::cout<<"\n";

        auto self=shared_from_this();
        send(_data, buffertransfered);
        memset(_data, 0, _max_legth);
        _socket.async_read_some(boost::asio::buffer(_data,_max_legth),std::bind(
            &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
        // boost::asio::async_write(_socket,boost::asio::buffer(_data,buffertransfered),
        // std::bind(&csession::handle_write,self,std::placeholders::_1));
    }else{
        _cserver->clear_csession(_uuid);
        std::cout<<"read error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    }
}

void csession::handle_write(const boost::system::error_code& ec){
    // if(!ec){
    //     memset(_data, 0, _max_legth);
    //     auto self=shared_from_this();
    //     _socket.async_read_some(boost::asio::buffer(_data,_max_legth),
    // std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
    // }else {
    //     _cserver->clear_csession(_uuid);
    //     std::cout<<"write error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    // }
    if(!ec){
        auto self=shared_from_this();
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_queue.pop();
        if(_send_queue.empty()){
            _send_padding=false;
            return;
        }
        auto& new_msgnode=_send_queue.front();
        boost::asio::async_write(_socket,boost::asio::buffer(new_msgnode->_data,new_msgnode->_max_length),
        std::bind(&csession::handle_write,self,std::placeholders::_1));
    }else {
            _cserver->clear_csession(_uuid);
            std::cout<<"write error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    }
}
