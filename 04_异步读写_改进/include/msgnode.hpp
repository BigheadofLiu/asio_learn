#pragma once
#include "csession.hpp"
#include <cstddef>
#include <cstring>

class msgnode{
    public:
    friend csession;
    msgnode(const char* msg,size_t total_len);
    ~msgnode();
    private:
    enum{_max_size=2014};
    size_t _cur_length;
    size_t _max_length;
    char* _data;
};
