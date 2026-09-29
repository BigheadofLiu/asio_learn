#include "cclient.hpp"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iterator>
//使用前面的 sync 客户端

namespace asio=boost::asio;
namespace ip=boost::asio::ip;
#define MAX_LENGTH 1024*2
#define HEAD_LENGTH sizeof(size_t)

int main(){
    try
    {
        asio::io_context ioc;
        ip::tcp::socket sock(ioc);
        ip::tcp::endpoint server_ep(ip::make_address("127.0.0.1"),8899);
        
        boost::system::error_code ec=boost::asio::error::host_not_found;
        sock.connect(server_ep,ec);
        if (ec)
        {
            std::cout<<"connect failed,code is:"<<ec.value()<<" "<<"error message is:"<<ec.message();
            return 0;
        }
        std::cout<<"enter message:";
        char request[MAX_LENGTH] {0};
        std::cin.getline(request,MAX_LENGTH);
        size_t request_length=strlen(request);

        char send_data[MAX_LENGTH + HEAD_LENGTH] {0};
        memcpy(send_data, &request_length, HEAD_LENGTH);
        memcpy(send_data + HEAD_LENGTH, request, request_length);
        asio::write(sock, asio::buffer(send_data, request_length + HEAD_LENGTH));
        // asio::write(sock,asio::buffer(request,request_length));
        char reply_head[HEAD_LENGTH]{0};
        // char reply[MAX_LENGTH] {};

        //echo服务器标准写法
        size_t reply_length=asio::read(sock,asio::buffer(reply_head,HEAD_LENGTH));
        size_t msglen=0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        char msg[MAX_LENGTH] {0};
        size_t msg_length=asio::read(sock,asio::buffer(msg,msglen));
        
        std::cout<<"reply is:";
        std::cout.write(msg,msglen)<<std::endl;
        std::cout<<"reply length is:"<<msglen<<"\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}

