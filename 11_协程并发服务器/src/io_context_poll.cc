#include "io_context_poll.hpp"
#include <iostream>
#include <thread>

using ioc=boost::asio::io_context;
io_context_poll::io_context_poll():
    _core(std::thread::hardware_concurrency()),
    _iocs(_core),
    _next_ioc(0){
        _work_guards.reserve(_core);
        _threads.reserve(_core);
        for(auto i=0;i<_core;i++){
            _work_guards.emplace_back(_iocs[i].get_executor());
            _threads.emplace_back([this,i](){
            _iocs[i].run();
            });
        }
}

io_context_poll::~io_context_poll(){
    std::cout<<"destruct io_context_poll"<<std::endl;
}

ioc& io_context_poll::get_ioc_context(){
    if(_next_ioc>=_core){
        _next_ioc=0;
    }
    auto& ioc=_iocs[_next_ioc];
    _next_ioc++;
    return ioc;
}

void io_context_poll::stop(){
    for(auto& work:_work_guards){
        work.reset();
    }
    for(auto& ioc:_iocs){
        ioc.stop();
    }
    for(auto& t:_threads){
        t.join();
    }
}