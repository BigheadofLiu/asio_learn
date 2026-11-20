#pragma once
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <string>

class session;
class server{
    using ioc=boost::asio::io_context;
    friend class session;
public:
    server(ioc& ioc,unsigned short port);
    ~server()=default;
    void clear_session(const std::string);
    void start_accept();
    void handle_accept(std::shared_ptr<session> new_session,const boost::system::error_code& ec);
private:
    ioc& _ioc;
    unsigned short _port;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::mutex _mutex;
    std::map<std::string,std::shared_ptr<session>> _sessions;
};