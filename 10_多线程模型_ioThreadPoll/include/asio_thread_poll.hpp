#pragma once
#include "singleton.hpp"
#include <boost/asio.hpp>
#include <cstddef>
#include <thread>
#include <vector>

class asio_thread_poll:public singleton<asio_thread_poll>{
    friend singleton<asio_thread_poll>;
public:
    using work_type=boost::asio::io_context::executor_type;
    
    ~asio_thread_poll();
    asio_thread_poll (const asio_thread_poll&)=delete;
    asio_thread_poll& operator=(const asio_thread_poll&)=delete;
    boost::asio::io_context& get_ioc_context();
    void stop();
private:
    asio_thread_poll();
    const std::size_t _size;
    boost::asio::io_context _ioc;  //共用一个ioc
    boost::asio::executor_work_guard<work_type> _work; //替代 unique_ptr
    std::vector<std::thread> _threads_v;
};