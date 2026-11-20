#include "server.hpp"
#include "session.hpp"
#include "io_context_poll.hpp"
#include <iostream>
#include <memory>
#include <utility>

server::server(boost::asio::io_context& ioc,unsigned short port)
    :_ioc(ioc),_port(port),
    _acceptor(ioc,{boost::asio::ip::tcp::v4(),_port}){
    std::cout<<"8899端口开始监听\n";
    start_accept();
}

void server::clear_session(const std::string uuid){
    _sessions.erase(uuid);
}

void server::start_accept(){
    auto& ioc=io_context_poll::get_instance()->get_ioc_context();
    //使用io_services_poll中得到的 ioc
    auto new_session=std::make_shared<session>(ioc,this);  
    _acceptor.async_accept(new_session->get_socket(),std::bind(&server::handle_accept,this,new_session,std::placeholders::_1));
}

void server::handle_accept(std::shared_ptr<session> new_session,const boost::system::error_code& ec){
    if(!ec){
        std::cout<<"new client connected"<<"\n";
        std::cout<<"客户端ip:"<<new_session->get_socket().remote_endpoint().address().to_string()<<" "<<
        "客户端 port："<<new_session->get_socket().remote_endpoint().port()<<"\n";
        _sessions.insert(std::make_pair(new_session->get_uuid(),new_session ));
        new_session->start();
    }else { 
        std::cout<<"accept error:"<<" "<<ec.value()<<" "<<ec.message();
    }
    start_accept();
}
