#include "msg_node.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/endian.hpp>

msg_node::msg_node(std::size_t total_length):
    _current_length(0),
    _total_length(total_length),
    _data(_total_length){

}

void msg_node::clear(){
    std::fill(_data.begin(), _data.end(), 0);
    _current_length=0;
}

void msg_node::set_send_data(const char* msg,std::uint16_t net_msg_id,std::uint32_t net_msg_length,std::size_t local_msg_length){
    std::memcpy(_data.data(), &net_msg_id, constants::msg_id_length);
    std::memcpy(_data.data()+constants::msg_id_length,&net_msg_length,constants::msg_head_length);
    std::memcpy(_data.data()+constants::msg_head_total_length, msg,local_msg_length);
}

msg_node::~msg_node()=default;

recv_msgnode::recv_msgnode(std::size_t total_length,std::uint16_t msg_id):
    msg_node(total_length),
    _msg_id(msg_id){

}

send_msgnode::send_msgnode(const char* msg,std::size_t total_length,std::uint16_t msg_id):
    msg_node(total_length+constants::msg_head_total_length),
    _msg_id(msg_id){
        auto net_msg_id=boost::endian::native_to_big(msg_id);
        auto net_msg_length=boost::endian::native_to_big(static_cast<uint32_t>(total_length));
        set_send_data(msg, net_msg_id, net_msg_length,total_length);
}