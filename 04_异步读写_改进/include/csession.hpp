#pragma once
#include "msgnode.hpp"
#include <boost/asio.hpp>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>


class cserver;
// #define MAX_LENGTH 1024;

class csession:public std::enable_shared_from_this<csession>{
    public:
    csession(boost::asio::io_context& ioc,cserver* cserver);
    void start();
    void send(char* msg,size_t total_length);
    boost::asio::ip::tcp::socket& get_socket();
    const std::string& get_uuid();

    private:
    boost::asio::ip::tcp::socket _socket;
    cserver* _cserver;
    std::string _uuid;
    enum{_max_legth=1024};
    char _data[_max_legth];
    void handle_read(const boost::system::error_code& ec,size_t buffertransfered);
    void handle_write( const boost::system::error_code& ec);
    std::queue<std::shared_ptr<msgnode>> _send_queue;
    std::queue<std::shared_ptr<msgnode>> _recv_queue;
    bool _recv_padding;
    bool _send_padding;
    std::mutex _send_lock;
    std::mutex _recv_lock;
};
