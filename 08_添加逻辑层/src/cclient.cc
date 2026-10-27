#include "cclient.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <boost/endian.hpp>
#include <condition_variable>
#include <mutex>
#include <random>
#include <time.h>

#define MAX_LENGTH 1024*2
#define HEAD_LENGTH sizeof(uint32_t) 
#define ID_LENGTH sizeof(uint16_t)
#define HEAD_TOTAL_LENGTH (HEAD_LENGTH + ID_LENGTH)

namespace asio=boost::asio;
namespace ip=boost::asio::ip;

enum msg_id:uint16_t{
    echo=1001,
    login=1002,
    chat=1003,
    transfrer=1004
};

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
        std::mutex mutex;
        std::condition_variable cv;
        bool can_send=true;

        std::thread send_thread([&sock,&mutex,&cv,&can_send]{
            for(;;){
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock,[&]{
                        return can_send;
                    });
                }
                std::cout<<"enter message:";
                char request[MAX_LENGTH] {0};
                std::cin.getline(request,MAX_LENGTH);
                uint16_t local_msg_id= msg_id::login;
                uint32_t local_length=strlen(request); //本机长度
                uint16_t network_msg_id=boost::endian::native_to_big(local_msg_id);
                uint32_t network_length=boost::endian::native_to_big(local_length);  //网络长度
                char send_data[MAX_LENGTH+HEAD_TOTAL_LENGTH]{0};
                memcpy(send_data,&network_msg_id,ID_LENGTH);
                memcpy(send_data+ID_LENGTH, &network_length, HEAD_LENGTH);
                memcpy(send_data + HEAD_TOTAL_LENGTH, request, local_length);
                asio::write(sock, asio::buffer(send_data, local_length + HEAD_TOTAL_LENGTH));
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    can_send=false;
                }
                cv.notify_one();
            }
        });

        std::thread recv_thread([&]{
            for(;;){
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock, [&] {
                    return !can_send;
                    });
                }
                char reply_head[HEAD_TOTAL_LENGTH] {0};
                //读消息头
                asio::read(sock,asio::buffer(reply_head,HEAD_TOTAL_LENGTH));
                uint16_t network_msg_id {0};
                uint32_t network_length=0;
                memcpy(&network_msg_id,reply_head,ID_LENGTH );
                memcpy(&network_length, reply_head+ID_LENGTH, HEAD_LENGTH);
                uint16_t local_msg_id=boost::endian::big_to_native(network_msg_id);
                uint32_t local_length=boost::endian::big_to_native(network_length);
                if(local_length>MAX_LENGTH){
                    throw std::runtime_error("reply too large");
                }
                //读消息正文
                char msg[MAX_LENGTH] {0};
                size_t msg_length=asio::read(sock,asio::buffer(msg,local_length));
                std::cout<<"reply is:";
                std::cout.write(msg,msg_length)<<std::endl;
                std::cout<<"reply length is:"<<local_length<<std::endl;
                std::cout<<"msg id is:"<<local_msg_id<<std::endl;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    can_send = true; // 轮到发送线程
                }
                cv.notify_one();
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

