#include "session.hpp"
#include "const.hpp"
#include "server.hpp"
#include "msg_node.hpp"
#include "logic_system.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
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
#include <boost/asio.hpp>

session::session(boost::asio::io_context& ioc,server* server)
    :_socket(ioc),
    _server(server),
    _recv_padding(false),
    _send_padding(false),
    _strand(ioc.get_executor()),
    _head_status(false),
    _recv_head_node(std::make_shared<msg_node>(constants::msg_head_total_length)){
    auto uuid=boost::uuids::random_generator()();
    _uuid=boost::uuids::to_string(uuid);
}

void session::start(){
    auto self=shared_from_this();
    boost::asio::co_spawn(_strand,read_loop(self),boost::asio::detached);
    // boost::asio::co_spawn(_socket.get_executor(),read_loop(self),boost::asio::detached);
}

boost::asio::awaitable<void> session::read_loop(std::shared_ptr<session> self){
    try {
        for(;;){
            self->_recv_head_node->clear();
            //接收消息头部
            co_await boost::asio::async_read(
                self->_socket,
                boost::asio::buffer(self->_recv_head_node->_data,constants::msg_head_total_length),
                boost::asio::use_awaitable
            );
            //处理消息头部
            uint16_t network_id = 0;
            uint32_t network_length =0;
            std::memcpy(&network_id,self->_recv_head_node->_data.data(),constants::msg_id_length);
            memcpy(&network_length, self->_recv_head_node->_data.data()+constants::msg_id_length, constants::msg_head_total_length-constants::msg_id_length);
            const auto local_length=boost::endian::big_to_native(network_length);
            const auto local_id=boost::endian::big_to_native(network_id);
            std::cout<<"json_data length is:"<<local_length<<","<<"data id is:"<<local_id<<std::endl;
            if(local_length>constants::msg_body_max_size){
                std::cout<<"invalid data length:"<<local_length<<std::endl;
                _server->clear_session(_uuid);
                co_return;
            }
            //接收消息正文
            self->_recv_msg_node=std::make_shared<recv_msgnode>(local_length,local_id);
            const auto byte_transferred=co_await boost::asio::async_read(
                self->_socket,
                boost::asio::buffer(self->_recv_msg_node->_data,local_length),
                boost::asio::use_awaitable
            );
            //处理正文消息
            self->_recv_msg_node->_current_length += byte_transferred;
            logic_system::get_instance()->post_logic_msg_to_queue(std::make_shared<logic_node>(self,self->_recv_msg_node));
        }
    } catch (const boost::system::system_error& e) {
    if (e.code() == boost::asio::error::eof ||
        e.code() == boost::asio::error::connection_reset) {
        std::cout << "client disconnected\n";
    } else {
        std::cout << e.what() << '\n';
    }
    } catch (const std::exception& e) {
    std::cout << e.what() << '\n';
    }
    self->_server->clear_session(self->_uuid);
    co_return;
}

void session::send(const char* msg,size_t total_length,uint16_t msg_id){
    //指定消息加入发送队列
    std::lock_guard<std::mutex> send_locker(_send_lock);
    //限制队列大小
    auto send_queue_size=_send_queue.size();
    if(send_queue_size>constants::send_queue_max_size){
        std::cout<<"session:"<<_uuid<<" "<<"send queue is fulled,"<<"size is:"<<send_queue_size<<std::endl;
        return;
    }
    _send_queue.push(std::make_shared<send_msgnode>(msg,total_length,msg_id));
    if(_send_padding){
        return;
    }
    _send_padding=true;
    auto& first_msg_node=_send_queue.front();
    auto self=shared_from_this();
    //从发送队列 取出消息发送
    boost::asio::async_write(_socket,boost::asio::buffer(first_msg_node->_data,first_msg_node->_total_length),
        boost::asio::bind_executor(_strand,std::bind(&session::handle_write,self,std::placeholders::_1))
    );
 }

void session::send(const std::string msg,uint16_t msg_id){
    //指定消息加入发送队列
    std::lock_guard<std::mutex> send_locker(_send_lock);
    //限制队列大小
    auto send_queue_size=_send_queue.size();
    if(send_queue_size>constants::send_queue_max_size){
        std::cout<<"session:"<<_uuid<<" "<<"send queue is fulled,"<<"size is:"<<send_queue_size<<std::endl;
        return;
    }
    _send_queue.push(std::make_shared<send_msgnode>(msg.c_str(),msg.length() ,msg_id));
    if(_send_padding){
        return;
    }
    _send_padding=true;
    auto& first_msg_node=_send_queue.front();
    auto self=shared_from_this();
    //从发送队列 取出消息发送
    boost::asio::async_write(_socket,boost::asio::buffer(first_msg_node->_data,first_msg_node->_total_length),
    // std::bind(&session::handle_write,self,std::placeholders::_1)
    boost::asio::bind_executor(_strand,std::bind(&session::handle_write,self,std::placeholders::_1))
    );
}

boost::asio::ip::tcp::socket& session::get_socket(){
    return _socket;
}

const std::string& session::get_uuid() const{
    return _uuid;
}

//测试粘包使用
void session::print_recv_data(char* data,int length){
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
void session::handle_read_head_total(const boost::system::error_code& ec,size_t byte_transfered){
    //先处理头部
    if(!ec){
        if(byte_transfered != constants::msg_head_total_length){
            std::cout<<"invalid head length:"<<byte_transfered<<std::endl;
            _server->clear_session(_uuid);
            return;
        }
        // std::memcpy(_recv_head_node->_data.data(), _data, byte_transfered); //不用_data 避免重复拷贝
        uint16_t network_id = 0;
        uint32_t network_length =0;
        std::memcpy(&network_id,_recv_head_node->_data.data(),constants::msg_id_length);
        memcpy(&network_length, _recv_head_node->_data.data()+constants::msg_id_length, byte_transfered-constants::msg_id_length);
        auto local_length=boost::endian::big_to_native(network_length);
        auto local_id=boost::endian::big_to_native(network_id);
        std::cout<<"json_data length is:"<<local_length<<","<<"data id is:"<<local_id<<std::endl;
        if(local_length>constants::msg_body_max_size){
            std::cout<<"invalid data length:"<<local_length<<std::endl;
            _server->clear_session(_uuid);
            return;
        }
        //头部处理完毕 处理正文
        auto self=shared_from_this();
        _recv_msg_node=std::make_shared<recv_msgnode>(local_length,local_id);
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(_recv_msg_node->_data,local_length),
            // std::bind(&session::handle_read_msg,self,std::placeholders::_1,std::placeholders::_2)
            boost::asio::bind_executor(_strand,std::bind(&session::handle_read_msg,self,std::placeholders::_1,std::placeholders::_2))
        );
    }else {
        std::cout<<"read head error"<<"value:"<<ec.value()<<"message:"<<ec.message()<<std::endl;
        _server->clear_session(_uuid);
        return;
    }
}
void session::handle_read_msg(const boost::system::error_code& ec,size_t byte_transfered){
    if(ec){
        if(ec == boost::asio::error::eof){
            std::cout<<"client close connect"<<std::endl;
        }else {
            std::cout<<"read msg error:"<<" "<<ec.value()<<" "<<ec.message()<<std::endl;
        }
        _server->clear_session(_uuid);
        return;
    }
    if(byte_transfered != _recv_msg_node->_data.size()){
        std::cout<<"invalid msg length:"<<byte_transfered<<std::endl;
        _server->clear_session(_uuid);
        return;
    }
    //改用逻辑层投递消息
    _recv_msg_node->_current_length += byte_transfered;
    auto self=shared_from_this();
    logic_system::get_instance()->post_logic_msg_to_queue(std::make_shared<logic_node>(self,_recv_msg_node));
    //处理下一个头文件
    _recv_head_node->clear();
    boost::asio::async_read(
        _socket,
        boost::asio::buffer(_recv_head_node->_data,constants::msg_head_total_length),
        // std::bind(&session::handle_read_head_total,self,std::placeholders::_1,std::placeholders::_2)
        boost::asio::bind_executor(_strand,std::bind(&session::handle_read_head_total,self,std::placeholders::_1,std::placeholders::_2))
    );
}

void session::handle_write(const boost::system::error_code& ec){
    if(!ec){
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_queue.pop();
        if(_send_queue.empty()){
            _send_padding=false;
            return;
        }
        auto self=shared_from_this();
        auto& next_msg_node=_send_queue.front();
        boost::asio::async_write(_socket,boost::asio::buffer(next_msg_node->_data,next_msg_node->_total_length),
        // std::bind(&session::handle_write,self,std::placeholders::_1)
        boost::asio::bind_executor(_strand,std::bind(&session::handle_write,self,std::placeholders::_1))
        );
    }else{
            _server->clear_session(_uuid);
            std::cout<<"write error:"<<" "<<ec.value()<<" "<<ec.message()<<std::endl;
    }
}
