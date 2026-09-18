#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "Session.hpp"

class Server{
    public:
    Server(boost::asio::io_context& ioc,unsigned short port);
    private:
    void start_accept();
    void handle_accept(std::shared_ptr<Session> new_session/*Session* new_session*/,const boost::system::error_code& ec);
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::acceptor _acc;
};