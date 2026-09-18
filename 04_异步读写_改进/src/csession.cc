#include "csession.hpp"
#include "cserver.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>
#include <functional>
#include <iostream>

csession::csession(boost::asio::io_context& ioc,cserver* cserver):_socket(ioc),_cserver(cserver){
    auto uuid=boost::uuids::random_generator()();
    _uuid=boost::uuids::to_string(uuid);
}

void csession::start(){
    memset(_data,0,_max_legth);
   auto self=shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data,_max_legth),
    std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
}

boost::asio::ip::tcp::socket& csession::get_socket(){
    return _socket;
}

const std::string& csession::get_uuid(){
    return _uuid;
}

void csession::handle_read(const boost::system::error_code& ec,size_t buffertransfered){
    if(ec== boost::asio::error::eof){
        std::cout<<"ip:"<<_socket.remote_endpoint().address()<<" "<<"port："<<_socket.remote_endpoint().port()<<" ";
        std::cout<<"client close connect"<<"\n";
        _cserver->clear_csession(_uuid);
        return;
    }
    if(!ec){
        std::cout<<"server recived massage is:";
        std::cout.write(_data, buffertransfered);
        std::cout<<"\n";

        auto self=shared_from_this();
        boost::asio::async_write(_socket,boost::asio::buffer(_data,buffertransfered),
        std::bind(&csession::handle_write,self,std::placeholders::_1));
    }else{
        _cserver->clear_csession(_uuid);
        std::cout<<"read error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    }
}

void csession::handle_write(const boost::system::error_code& ec){
    if(!ec){
        memset(_data, 0, _max_legth);
        auto self=shared_from_this();
        _socket.async_read_some(boost::asio::buffer(_data,_max_legth),
    std::bind(&csession::handle_read,self,std::placeholders::_1,std::placeholders::_2));
    }else {
        _cserver->clear_csession(_uuid);
        std::cout<<"write error:"<<" "<<ec.value()<<" "<<ec.message()<<"\n";
    }
}
