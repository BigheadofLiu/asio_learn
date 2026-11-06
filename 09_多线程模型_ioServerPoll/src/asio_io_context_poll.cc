#include "asio_io_context_poll.hpp"
#include <cstddef>
#include <iostream>
#include <thread>

asio_io_context_poll::asio_io_context_poll():
    _size(std::thread::hardware_concurrency()),
    _io_contexts(_size),  //ioc_context 提供有参 相当于在 vector 构造一个 io_context
    _next_io_service(0)
    // _work_guards_ptrs(size)  executor_work_guard 不提供无参构造函数 不能这么写
    
{   //放在函数体构造
    _work_guards.reserve(_size);  //预留空间
    // _work_guard_ptrs(size);
    for(std::size_t i=0;i<_size;++i){ 
        _work_guards.emplace_back(_io_contexts[i].get_executor()); //直接在 vector 内部构造 避免拷贝
        // _work_guard_ptrs.emplace_back(boost::asio::executor_work_guard<_work>(_io_contexts[i].get_executor()));
    }
    //添加进线程
    _threads.reserve(_size);
    for(size_t i=0;i<_size;++i){
        _threads.emplace_back([this,i](){
            _io_contexts[i].run();
        });
    }
}

boost::asio::io_context& asio_io_context_poll::get_io_service(){
    //这种写法不太理解
    // auto& service=_io_contexts[_next_io_service++];
    // if(_next_io_service==_io_contexts.size()){
    //     _next_io_service=0;
    // }
    // return service;

    //我理解的写法
    if(_next_io_service>=_io_contexts.size()){
        _next_io_service=0;
    }
        auto& service=_io_contexts[_next_io_service];
        _next_io_service++;
        return service;
}

void asio_io_context_poll::stop(){
    //工作任务停止
    for(auto& work_guard:_work_guards){
        work_guard.reset();
    }
    //防止服务器无法强退 ioc 也要停止
    for(auto& ioc:_io_contexts){
        ioc.stop();
    }
    //退出前 线程任务需要执行完毕
    for(auto& thread:_threads){
        thread.join();
    }
}

asio_io_context_poll::~asio_io_context_poll(){
    std::cout<<"asio_io_service_poll has distruct"<<std::endl;
}