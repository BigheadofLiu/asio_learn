#include "client.hpp"

namespace asio=boost::asio;
namespace ip=boost::asio::ip;

const int MAX_LENGTH = 1024;
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
        char request[MAX_LENGTH] {};
        std::cin.getline(request,MAX_LENGTH);
        size_t request_length=strlen(request);
        asio::write(sock,asio::buffer(request,request_length));

        char reply[MAX_LENGTH] {};
        //echo服务器标准写法
        size_t reply_length=asio::read(sock,asio::buffer(reply,request_length));
        std::cout<<"reply is:";
        std::cout.write(reply,reply_length)<<std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}


