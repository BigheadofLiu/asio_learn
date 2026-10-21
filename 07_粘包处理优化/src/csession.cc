#include "csession.hpp"
#include "cserver.hpp"
#include "msgnode.hpp"
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <sys/socket.h>
#include <iomanip>
#include <boost/endian/conversion.hpp>
#include <unistd.h>
#include <unistd.h>

csession::csession(boost::asio::io_context& ioc,cserver* cserver)
    :_socket(ioc),
    _cserver(cserver),
    _recv_padding(false),
    _send_padding(false),
    _head_status(false),
    _recv_head_node(std::make_shared<msgnode>(_head_length)){
    auto uuid=boost::uuids::random_generator()();
    _uuid=boost::uuids::to_string(uuid);
}

void csession::start(){
    // memset(_data,0,_max_length); 
    //不再使用_data 减少拷贝次数
    _recv_head_node->clear();
    auto self=shared_from_this(); 
    //改用boost::asio::async_read 
    //先读头部
    boost::asio::async_read(
        _socket,
        boost::asio::buffer(_recv_head_node->_data,_head_length),
        std::bind(&csession::handle_read_head,self,std::placeholders::_1,std::placeholders::_2));
}

void csession::send(const char* msg,size_t total_length){
    //指定消息加入发送队列
    std::lock_guard<std::mutex> send_locker(_send_lock);
    //限制队列大小
    auto send_queue_size=_send_queue.size();
    if(send_queue_size>_queue_max_size){
        std::cout<<"session:"<<_uuid<<" "<<"send queue is fulled,"<<"size is:"<<send_queue_size<<std::endl;
        return;
    }
    _send_queue.push(std::make_shared<msgnode>(msg,total_length));
    if(_send_padding){
        return;
    }
    _send_padding=true;
    auto& first_msgnode=_send_queue.front();
    auto self=shared_from_this();
    //从发送队列 取出消息发送
    boost::asio::async_write(_socket,boost::asio::buffer(first_msgnode->_data,first_msgnode->_max_length),
    std::bind(&csession::handle_write,self,std::placeholders::_1));
 }

boost::asio::ip::tcp::socket& csession::get_socket(){
    return _socket;
}

const std::string& csession::get_uuid() const{
    return _uuid;
}

//测试粘包使用
void csession::print_recv_data(char* data,int length){
    //写法完全看不懂
    std::stringstream ss;
    std::string result = "0x";
    for (int i = 0; i < length; i++) {
        std::string hexstr;
        ss << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << std::endl;
        ss >> hexstr;
        result += hexstr;
    }
    std::cout << "receive raw data is : " << result << std::endl;
}
//handle_read 改为handle_read_head handle_read_msg
void csession::handle_read_head(const boost::system::error_code& ec,size_t byte_transfered){
    //先处理头部
    if(!ec){
        if(byte_transfered != _head_length){
            std::cout<<"invalid head length:"<<byte_transfered<<std::endl;
            _cserver->clear_csession(_uuid);
            return;
        }
        // std::memcpy(_recv_head_node->_data.data(), _data, byte_transfered); //不用_data 避免重复拷贝
        uint32_t network_length =0;
        memcpy(&network_length, _recv_head_node->_data.data(), byte_transfered);
        auto local_length=boost::endian::big_to_native(network_length);
        std::cout<<"data length is:"<<local_length<<std::endl;
        if(local_length>_max_length){
            std::cout<<"invalid data length:"<<local_length<<std::endl;
            _cserver->clear_csession(_uuid);
            return;
        }
        //头部处理完毕 处理正文
        auto self=shared_from_this();
        _recv_msg_node=std::make_shared<msgnode>(local_length);
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(_recv_msg_node->_data,local_length),
            std::bind(&csession::handle_read_msg,self,std::placeholders::_1,std::placeholders::_2)
        );
    }else {
        std::cout<<"read head error"<<"value:"<<ec.value()<<"message:"<<ec.message()<<std::endl;
        _cserver->clear_csession(_uuid);
        return;
    }
 
}
void csession::handle_read_msg(const boost::system::error_code& ec,size_t byte_transfered){
    if(ec){
        if(ec == boost::asio::error::eof){
            std::cout<<"client close connect"<<std::endl;
        }else {
            std::cout<<"read msg error:"<<" "<<ec.value()<<" "<<ec.message()<<std::endl;
        }
        _cserver->clear_csession(_uuid);
        return;
    }
    if(byte_transfered != _recv_msg_node->_data.size()){
        std::cout<<"invalid msg length:"<<byte_transfered<<std::endl;
        _cserver->clear_csession(_uuid);
        return;
    }
    //没有使用 json protocbuf 
    std::cout<<"receive data is:";
    for(auto i:_recv_msg_node->_data){
        std::cout<<i;
    }
    std::cout<<std::endl;
    //写回
    csession::send(_recv_msg_node->_data.data(),byte_transfered);
    //处理下一个头文件
    auto self=shared_from_this();
    boost::asio::async_read(
        _socket,
        boost::asio::buffer(_recv_head_node->_data,_head_length),
        std::bind(&csession::handle_read_head,self,std::placeholders::_1,std::placeholders::_2)
    );
}

void csession::handle_write(const boost::system::error_code& ec){
    if(!ec){
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_queue.pop();
        if(_send_queue.empty()){
            _send_padding=false;
            return;
        }
        auto self=shared_from_this();
        auto& next_msgnode=_send_queue.front();
        boost::asio::async_write(_socket,boost::asio::buffer(next_msgnode->_data,next_msgnode->_max_length),
        std::bind(&csession::handle_write,self,std::placeholders::_1));
    }else{
            _cserver->clear_csession(_uuid);
            std::cout<<"write error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    }
}
