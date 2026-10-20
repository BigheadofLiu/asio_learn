#include "cclient.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <ostream>
#include <thread>
#include <boost/endian.hpp>
#include <json/json.h>
namespace asio=boost::asio;
namespace ip=boost::asio::ip;
#define MAX_LENGTH 1024*2
#define HEAD_LENGTH sizeof(uint32_t) 

//修改为使用 json传递消息正文
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
        std::thread send_thread([&sock]{
                for (int i=1001; i<=1020;i++) {
                    Json::Value root_1;
                    root_1["id"]=i;
                    root_1["data"]= "hello world!";
                    std::string request {};
                    request=root_1.toStyledString();

                    size_t request_length=request.length(); //本机长度
                    uint32_t network_length=boost::endian::native_to_big(static_cast<uint32_t>(request_length)); //网络长度
                    char send_data[MAX_LENGTH+HEAD_LENGTH] {0};
                    memcpy(send_data, &network_length, HEAD_LENGTH);
                    memcpy(send_data + HEAD_LENGTH, request.c_str(), request_length);
                    asio::write(sock, asio::buffer(send_data, request_length + HEAD_LENGTH));
                }
        });
        std::thread recv_thread([&sock]{
            for(;;){
                
                char reply_head[HEAD_LENGTH]{0};
                //读消息头
                asio::read(sock,asio::buffer(reply_head,HEAD_LENGTH));
                uint32_t network_length=0;
                memcpy(&network_length, reply_head, HEAD_LENGTH);
                uint32_t reply_length=boost::endian::big_to_native(network_length);
                //读消息正文
                char msg[MAX_LENGTH] {0};
                size_t msg_length=asio::read(sock,asio::buffer(msg,reply_length));

                Json::Value root_2;
                Json::Reader read;
                read.parse(msg,root_2);
                std::cout<<"reply is:";
                std::cout<<root_2["id"]<<" "<<root_2["data"]<<std::endl;
                std::cout<<"reply length is:"<<reply_length<<std::endl;
            }
        });
        send_thread.join();
        recv_thread.join();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}

