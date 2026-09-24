#pragma once
#include <cstddef>
#include <vector>
#include <memory>

class csession;

class msgnode{
    public:
    friend csession;
    msgnode(const char* msg,size_t total_len);
    msgnode(size_t total_len);
    void clear();
    private:
    enum{
        _max_size=1024*2,
        // _head_length=2
        _head_length=sizeof(std::size_t)
    };
    size_t _cur_length;
    size_t _max_length;
    // char* _data;
    std::vector<char> _data;  //裸指针改为 vector 避免手动释放
};
