#include "Server.hpp"
#include <functional>
#include <iostream>
#include <memory>

Server::Server(boost::asio::io_context& ioc,unsigned short port):_ioc(ioc),
_acc(ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)){
    std::cout<<"8899端口开始监听客户端"<<"\n";
    start_accept();
}
void Server::start_accept(){
    // auto new_session=new Session(_ioc);
    auto new_session=std::make_shared<Session>(_ioc); //智能指针的生存周期 影响回调函数的调用。。。
    // std::shared_ptr<Session> new_session(new Session(_ioc));
    _acc.async_accept(new_session->get_socket(),
    std::bind(&Server::handle_accept,this,new_session,std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> new_session/*Session* new_session*/,const boost::system::error_code& ec){
    if(!ec){
        std::cout<<"new client connected"<<"\n";
        std::cout<<"客户端ip:"<<new_session->get_socket().remote_endpoint().address().to_string()<<std::endl<<
        "客户端 port："<<new_session->get_socket().remote_endpoint().port()<<"\n";
        new_session->start();
    }else {
        std::cout<<"accept error,error code is:"<<ec.value()<<"message is:"<<ec.message();
    }
    start_accept();
}


