#include "cserver.hpp"
#include "csession.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

cserver::cserver(boost::asio::io_context& ioc,unsigned short port):_ioc(ioc),_port(port),
    _acc(ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),_port)){
        std::cout<<"8899端口开始监听\n";
        start_accept();
}

void cserver::clear_csession(const std::string uuid){
    _map_csessions.erase(uuid);
}

void cserver::start_accept(){
    auto new_session=std::make_shared<csession>(_ioc,this); 
    _acc.async_accept(new_session->get_socket(),std::bind(&cserver::handle_accept,this,new_session,std::placeholders::_1));
}

void cserver::handle_accept(std::shared_ptr<csession> new_session,const boost::system::error_code& ec){
    if(!ec){
        std::cout<<"new client connected"<<"\n";
        std::cout<<"客户端ip:"<<new_session->get_socket().remote_endpoint().address().to_string()<<" "<<
        "客户端 port："<<new_session->get_socket().remote_endpoint().port()<<"\n";

        new_session->start();
        _map_csessions.insert(std::make_pair(new_session->get_uuid(),new_session ));
    }else { 
        std::cout<<"accept error:"<<" "<<ec.value()<<" "<<ec.message();
    }
    start_accept();
}
