#include "csession.hpp"
#include "cserver.hpp"
#include "msgnode.hpp"
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
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
#include <json/json.h>

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
    memset(_data,0,_max_length);
    // std::fill(_data, _data+_max_legth, 0);
    auto self=shared_from_this();
    //读原始数据到临时数组 消息头+消息本身通过拆包拼出
    _socket.async_read_some(boost::asio::buffer(_data,_max_length),
    std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
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

void csession::handle_read(const boost::system::error_code& ec,size_t byte_transfered){
    //byte_transfered 由 asio 传入 代表本次async_read_some 读入的字节数
    if(ec){
        if(ec== boost::asio::error::eof){
        // std::cout<<"ip:"<<_socket.remote_endpoint().address()<<" "<<"port："<<_socket.remote_endpoint().port()<<" ";
             std::cout<<"client close connect"<<"\n";
         }else {
            std::cout<<"read error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
         }
        _cserver->clear_csession(_uuid);
        return;
    }
    size_t copy_length = 0;  //已处理长度
    auto self=shared_from_this();
    while (byte_transfered > 0) {
        //拼接 消息头+消息正文
        if(!_head_status){
            //消息头部处理
            if(byte_transfered+_recv_head_node->_cur_length<_head_length){
                //消息头部长度不足
                memcpy(_recv_head_node->_data.data()+_recv_head_node->_cur_length, _data + copy_length,byte_transfered);
                _recv_head_node->_cur_length+=byte_transfered;
                std::fill(_data,_data+_max_length,0);
                // ::memset(_data, 0, _max_length);
                auto self=shared_from_this();
                _socket.async_read_some(boost::asio::buffer(_data,_max_length),std::bind(
                    &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
                return;
            }
            //头部完整
            size_t head_remain=_head_length-_recv_head_node->_cur_length;  //头部长度
            memcpy(_recv_head_node->_data.data()+_recv_head_node->_cur_length, _data+copy_length, head_remain);
            copy_length += head_remain;
            byte_transfered -= head_remain;  //剩余未处理数据长度(消息正文)

            uint32_t network_length=0;   //解析消息长度
            memcpy(&network_length, _recv_head_node->_data.data(), _head_length);
            auto data_length=boost::endian::big_to_native(network_length);
            std::cout<<"data_length is:"<<data_length<<"\n";

            if(data_length>_max_length){
                //长度非法
                std::cout<<"invalid length:"<<data_length<<"\n";
                _cserver->clear_csession(_uuid);
                return;
            }
            //开始处理消息正文
            _recv_msg_node=std::make_shared<msgnode>(data_length);  //用于接收消息正文
            //除去消息头长度的 byte_transfered 就是消息正文长度
            if(byte_transfered<data_length){
                //消息正文长度不足
                memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length,_data+copy_length,byte_transfered);
                _recv_msg_node->_cur_length+=byte_transfered;
                // copy_length += byte_transfered; 
                //继续接收消息正文
                _head_status=true;  //true 表示消息头接收完毕 正文未接收完毕
                std::fill(_data,_data+_max_length,0);
                _socket.async_read_some(boost::asio::buffer(_data,_max_length),std::bind(
                    &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
                return;
            }
            //消息正文长度充足
            memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length,_data+copy_length,data_length);
            _recv_msg_node->_cur_length+=data_length;
            copy_length+=data_length;
            byte_transfered-=data_length;
            Json::Value recv_json;
            Json::Reader read;
            read.parse(_recv_msg_node->_data.data(),recv_json);
            std::cout<<"receive data is:";
            std::cout<<"id:"<<recv_json["id"]<<" "<<"data:"<<recv_json["data"]<<std::endl;
            //这下面有点脱裤放屁啊
            Json::Value send_json;
            send_json["id"]=recv_json["id"];
            send_json["data"]=recv_json["data"];
            std::string return_msg=send_json.toStyledString();
            send(return_msg.c_str(),return_msg.length());
            _head_status=false;
            _recv_head_node->clear();
            if(byte_transfered<=0){
                std::fill(_data, _data+_max_length, 0);
                _socket.async_read_some(boost::asio::buffer(_data,_max_length),std::bind(
                    &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
                return;
            }
            continue;
        }
        //消息头部处理完毕 处理未接收完消息正文
        size_t remain_msg=_recv_msg_node->_max_length-_recv_msg_node->_cur_length;
        if(byte_transfered<remain_msg){
            //不足消息正文长度
            memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length, _data+copy_length, byte_transfered);
            _recv_msg_node->_cur_length += byte_transfered;
            std::fill(_data, _data+_max_length, 0);
            _socket.async_read_some(boost::asio::buffer(_data,_max_length),
            std::bind(
                &csession::handle_read,
                self,
                std::placeholders::_1,
                std::placeholders::_2
            ));
            return;
        }
        //满足消息正文长度
        memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length,_data+copy_length,remain_msg);
        _recv_msg_node->_cur_length+=remain_msg;
        byte_transfered -= remain_msg;
        copy_length += remain_msg;
        Json::Value recv_json;
        Json::Reader read;
        read.parse(_recv_msg_node->_data.data(),recv_json);
        std::cout<<"receive data is:";
        std::cout<<"id:"<<recv_json["id"]<<" "<<"data:"<<recv_json["data"]<<std::endl;
        //这下面有点脱裤放屁啊
        Json::Value send_json;
        send_json["id"]=recv_json["id"];
        send_json["dada"]=recv_json["data"];
        std::string return_msg=send_json.toStyledString();
        send(return_msg.c_str(),return_msg.length());
        _head_status=false;
        _recv_head_node->clear();
        if(byte_transfered<=0){
            std::fill(_data, _data+_max_length, 0);
            _socket.async_read_some(boost::asio::buffer(_data,_max_length),std::bind(
                &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
            return;
        }
        continue;
    }
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
