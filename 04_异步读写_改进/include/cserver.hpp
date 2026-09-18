#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <map>

class csession;

class cserver{
    public:
    cserver(boost::asio::io_context& ioc,unsigned short port);
    void clear_csession(const std::string);  //清理指定 session
    private:
    void start_accept();
    void handle_accept(std::shared_ptr<csession> new_session,const boost::system::error_code& ec);
    unsigned short _port;
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::acceptor _acc;
    std::map<std::string, std::shared_ptr<csession>> _map_csessions; //map 管理 session
};
