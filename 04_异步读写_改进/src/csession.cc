#include "csession.hpp"
#include "cserver.hpp"
#include "msgnode.hpp"
#include <algorithm>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sys/socket.h>

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
    //读原始数据到临时数组 消息头+消息本身
    _socket.async_read_some(boost::asio::buffer(_data,_max_length),
    std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
}

void csession::send(const char* msg,size_t total_length){
    //从发送队列 取消息发送
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

const std::string& csession::get_uuid() const{
    return _uuid;
}

// void csession::handle_read(const boost::system::error_code& ec,size_t buffertransfered){
//     if(ec== boost::asio::error::eof){
//         // std::cout<<"ip:"<<_socket.remote_endpoint().address()<<" "<<"port："<<_socket.remote_endpoint().port()<<" ";
//         std::cout<<"client close connect"<<"\n";
//         _cserver->clear_csession(_uuid);
//         return;
//     }
//     if(!ec){
//         std::cout<<"server recived massage is:";
//         std::cout.write(_data, buffertransfered);
//         std::cout<<"\n";

//         auto self=shared_from_this();
//         send(_data, buffertransfered);

//         memset(_data, 0, _max_legth);
//         _socket.async_read_some(boost::asio::buffer(_data,_max_legth),std::bind(
//             &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
//         // boost::asio::async_write(_socket,boost::asio::buffer(_data,buffertransfered),
//         // std::bind(&csession::handle_write,self,std::placeholders::_1));
//     }else{
//         _cserver->clear_csession(_uuid);
//         std::cout<<"read error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
//     }
// }

void csession::handle_read(const boost::system::error_code& ec,size_t byte_transfered){
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
        if(!_head_status){
            //消息头部处理
            if(byte_transfered+_recv_head_node->_cur_length<_head_length){
                //消息头部不足
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
            byte_transfered -= head_remain;  //剩余未处理数据长度

            size_t data_length=0;   //解析消息长度
            memcpy(&data_length, _recv_head_node->_data.data(), _head_length);
            std::cout<<"data_length is:"<<data_length<<"\n";

            if(data_length>_max_length){
                //长度非法
                std::cout<<"invalid length:"<<data_length<<"\n";
                _cserver->clear_csession(_uuid);
                return;
            }
            
            _recv_msg_node=std::make_shared<msgnode>(data_length);  //用于接收消息正文
            if(byte_transfered<data_length){
                //消息正文不足
                memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length,_data+copy_length,byte_transfered);
                _recv_msg_node->_cur_length+=byte_transfered;

                std::fill(_data,_data+_max_length,0);
                _socket.async_read_some(boost::asio::buffer(_data,_max_length),std::bind(
                    &csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
                //头部处理完成
                _head_status=true;
                return;
            }
            //消息正文完整
            memcpy(_recv_msg_node->_data.data()+_recv_msg_node->_cur_length,_data+copy_length,data_length);
            _recv_msg_node->_cur_length+=data_length;
            copy_length+=data_length;
            byte_transfered-=data_length;
            // _recv_msg_node->_data[_recv_msg_node->_max_length-1]+='\0';
            std::cout<<"receive data is:";
            for(auto i:_recv_msg_node->_data){
                std::cout<<i;
            }
            std::cout<<std::endl;
            send(_recv_msg_node->_data.data(), _recv_msg_node->_max_length);
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

        std::cout<<"receive data is:";
        for(auto i:_recv_msg_node->_data){
            std::cout<<i;
        }
        std::cout<<std::endl;
        send(_recv_msg_node->_data.data(), _recv_msg_node->_max_length);
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
