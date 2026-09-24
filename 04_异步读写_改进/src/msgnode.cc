#include "msgnode.hpp"
#include <algorithm>
#include <cstring>

//发送数据 带头部
msgnode::msgnode(const char* msg, size_t total_len)
    : _cur_length(0),
      _max_length(total_len + _head_length),
      _data(_max_length) {
    std::memcpy(_data.data(), &total_len, _head_length);
    std::memcpy(_data.data() + _head_length, msg, total_len);
}

//接收数据 不带头部
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