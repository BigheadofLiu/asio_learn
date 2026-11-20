#pragma once
#include <boost/asio.hpp>
#include <cstddef>
#include <thread>
#include <vector>
#include "singleton.hpp"

class io_context_poll:public singleton<io_context_poll>{
    using ioc=boost::asio::io_context;
    using work=boost::asio::io_context::executor_type;
    using work_guard=boost::asio::executor_work_guard<work>;

    friend class singleton<io_context_poll>;
    public:
    void stop();
    ioc& get_ioc_context();
    ~io_context_poll();
    io_context_poll(const io_context_poll&)=delete;
    io_context_poll& operator=(const io_context_poll&)=delete;
    private:
    io_context_poll();
    const size_t _core;
    std::vector<std::thread> _threads;
    std::vector<ioc> _iocs;
    std::vector<work_guard> _work_guards;
    std::size_t _next_ioc;
};