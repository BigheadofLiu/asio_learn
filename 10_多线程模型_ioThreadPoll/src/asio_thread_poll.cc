#include "asio_thread_poll.hpp"
#include <iostream>
#include <ostream>
#include <thread>

asio_thread_poll::asio_thread_poll():_size(std::thread::hardware_concurrency()),
    _work(_ioc.get_executor())
{   
    _threads_v.reserve(_size);
    for (auto i=0;i<_size;i++) {
        _threads_v.emplace_back([this]()->auto{
            _ioc.run();
        });
    }
}

boost::asio::io_context& asio_thread_poll::get_ioc_context(){
    return _ioc;
}

void asio_thread_poll::stop(){
    _work.reset();
    _ioc.stop();
    for (auto& t:_threads_v ) {
        t.join();
    }
}

asio_thread_poll::~asio_thread_poll(){
    std::cout<<"asio_thread_poll has destroied"<<std::endl;
}