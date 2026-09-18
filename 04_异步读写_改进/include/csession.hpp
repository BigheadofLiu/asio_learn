#pragma once
#include <boost/asio.hpp>
#include <cstddef>
#include <memory>

class cserver;
// #define MAX_LENGTH 1024;

class csession:public std::enable_shared_from_this<csession>{
    public:
    csession(boost::asio::io_context& ioc,cserver* cserver);
    void start();
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
};
