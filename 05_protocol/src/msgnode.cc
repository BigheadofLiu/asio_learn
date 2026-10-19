#include "msgnode.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <boost/endian.hpp>
//发送数据 带头部
msgnode::msgnode(const char* msg, size_t total_len)
    : _cur_length(0),
    _max_length(total_len + _head_length),
    _data(_max_length) {
    uint32_t network_len=boost::endian::native_to_big(static_cast<uint32_t>(total_len)); //消息头改为网络序（大段）
    std::memcpy(_data.data(), &network_len, _head_length);
    std::memcpy(_data.data() + _head_length, msg, total_len);
}

//构造指定长度的消息节点 不带头部长度
msgnode::msgnode(size_t total_len)
    :_cur_length(0),
    _max_length(total_len),
    _data(_max_length){
}

//清除数据
void msgnode::clear(){
  // memset(_data, 0, _max_length);
  std::fill(_data.begin(), _data.end(), 0);
  _cur_length=0;
}