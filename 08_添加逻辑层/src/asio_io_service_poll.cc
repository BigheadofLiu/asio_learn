#include "asio_io_service_poll.hpp"
#include <cstddef>
#include <iostream>

asio_io_service_poll::asio_io_service_poll(std::size_t size):
    _io_services(size),
    _next_io_service(0)
    // _work_guards_ptrs(size)  executor_work_guard 不提供无参构造函数 不能这么写
    
{   //放在函数体构造
    _work_guards.reserve(size);
    // _work_guard_ptrs(size);
    for(std::size_t i=0;i<size;++i){ 
        _work_guards.emplace_back(_io_services[i].get_executor()); //直接在 vector 内部构造 避免拷贝
        // _work_guard_ptrs.emplace_back(boost::asio::executor_work_guard<_work>(_io_services[i].get_executor()));
    }
    for(size_t i=0;i<size;++i){
        _threads.emplace_back([this,i](){
            _io_services[i].run();
        });
    }
}

boost::asio::io_context& asio_io_service_poll::get_io_service(){
    //这种写法不太理解
    // auto& service=_io_services[_next_io_service++];
    // if(_next_io_service==_io_services.size()){
    //     _next_io_service=0;
    // }
    // return service;

    //我理解的写法
    if(_next_io_service>=_io_services.size()){
        _next_io_service=0;
    }
        auto& service=_io_services[_next_io_service];
        _next_io_service++;
        return service;
}

void asio_io_service_poll::stop(){
    for(auto& work_guard:_work_guards){
        work_guard.reset();
    }
    for(auto& thread:_threads){
        thread.join();
    }
}

asio_io_service_poll::~asio_io_service_poll(){
    std::cout<<"asio_io_service_poll has distruct"<<std::endl;
}