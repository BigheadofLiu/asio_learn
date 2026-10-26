#include "msgnode.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <boost/endian.hpp>
//发送数据 带头部
// msgnode::msgnode(const char* msg, size_t total_len)
//     : _cur_length(0),
//     _max_length(total_len + _head_length),
//     _data(_max_length) {
//     uint32_t network_len=boost::endian::native_to_big(static_cast<uint32_t>(total_len)); //消息头改为网络序（大段）
//     std::memcpy(_data.data(), &network_len, _head_length);
//     std::memcpy(_data.data() + _head_length, msg, total_len);
// }

//构造指定长度的消息节点 不带头部长度 接收数据
// msgnode::msgnode(size_t total_len)
//     :_cur_length(0),
//     _max_length(total_len),
//     _data(_max_length){
// }

//清除数据
void msgnode::clear(){
  // memset(_data, 0, _max_length);
  std::fill(_data.begin(), _data.end(), 0);
  _cur_length=0;
}

msgnode::msgnode(std::size_t total_length)
  :_cur_length(0),
  _max_length(total_length),
  _data(_max_length){
}
//接收信息
recv_msgnode::recv_msgnode(size_t total_length,uint16_t msg_id)
  :msgnode(total_length),
  _msg_id(msg_id){
}
//发送信息
send_msgnode::send_msgnode(const char* msg,size_t total_length,uint16_t msg_id)
  :msgnode(total_length + msgnode::_head_length + _id_length),
  _msg_id(msg_id){
    //解析 id
    uint16_t network_id=boost::endian::native_to_big(static_cast<uint16_t>(msg_id));
    std::memcpy(_data.data(),&network_id,_id_length);
    //解析消息长度
    uint32_t network_len=boost::endian::native_to_big(static_cast<uint32_t>(total_length)); //消息头改为网络序（大段）
    std::memcpy(_data.data()+_id_length, &network_len, _head_length);
    //剩余为消息正文
    std::memcpy(_data.data()+_id_length+_head_length, msg, total_length);
}
