#pragma once
#include <cstddef>
#include <memory>
#include <singleton.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <vector>

class asio_io_context_poll:public singleton<asio_io_context_poll>{
    friend singleton<asio_io_context_poll>;
public:
    // using _work=boost::asio::io_context::work; //新版本已取消
    using _work=boost::asio::io_context::executor_type;
    // using _work_ptr=std::unique_ptr<_work>;  //新版本已取消
    using _work_guard=boost::asio::executor_work_guard<_work>; //新版写法

    ~asio_io_context_poll();
    asio_io_context_poll(const asio_io_context_poll&)=delete;
    asio_io_context_poll& operator=(const asio_io_context_poll)=delete;
    boost::asio::io_context& get_io_service();
    void stop();
private:
    asio_io_context_poll();  
    const std::size_t _size;  //cpu 核数
    std::vector<boost::asio::io_context> _io_contexts;
    std::vector<_work_guard> _work_guards;
    std::vector<std::thread> _threads;
    std::size_t _next_io_service;
};