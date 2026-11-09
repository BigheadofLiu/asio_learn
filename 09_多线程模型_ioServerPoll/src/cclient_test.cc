#include "cclient.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
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
#include <json/json.h>
#include <vector>

#define MAX_LENGTH 1024*2
#define HEAD_LENGTH sizeof(uint32_t) 
#define ID_LENGTH sizeof(uint16_t)
#define HEAD_TOTAL_LENGTH (HEAD_LENGTH + ID_LENGTH)

namespace asio=boost::asio;
namespace ip=boost::asio::ip;

enum msg_id:uint16_t{
    _msg_hello_world = 1001,
    _msg_alive = 1002,
    _msg_chat = 1003
};
std::vector<std::thread> v_threads;
int main(){
    auto start_time=std::chrono::high_resolution_clock::now();
        for(auto i=0;i<100;i++){
            v_threads.emplace_back([]() ->auto{
                try {
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
                    for(auto j=0;j<500;j++){
                        uint16_t local_msg_id= msg_id::_msg_hello_world;
                        Json::Value root;
                        root["id"]=local_msg_id;
                        root["data"]="hello world";
                        std::string json_request=root.toStyledString();
                        uint32_t local_length=json_request.length(); //本机长度
                        uint16_t network_msg_id=boost::endian::native_to_big(local_msg_id);
                        uint32_t network_length=boost::endian::native_to_big(local_length);  //网络长度
                        char send_data[MAX_LENGTH+HEAD_TOTAL_LENGTH]{0};
                        memcpy(send_data,&network_msg_id,ID_LENGTH);
                        memcpy(send_data+ID_LENGTH, &network_length, HEAD_LENGTH);
                        memcpy(send_data + HEAD_TOTAL_LENGTH, json_request.data(), local_length);
                        asio::write(sock, asio::buffer(send_data, local_length + HEAD_TOTAL_LENGTH));

                        std::cout << "begin to receive..." << std::endl;
                        char reply_head[HEAD_TOTAL_LENGTH] {0};
                        //读消息头 id+length
                        asio::read(sock,asio::buffer(reply_head,HEAD_TOTAL_LENGTH));
                        uint16_t network_msg_id_read {0};
                        uint32_t network_length_read=0;
                        memcpy(&network_msg_id_read,reply_head,ID_LENGTH );
                        memcpy(&network_length_read, reply_head+ID_LENGTH, HEAD_LENGTH);
                        uint16_t local_msg_id_read=boost::endian::big_to_native(network_msg_id_read);
                        uint32_t local_length_read=boost::endian::big_to_native(network_length_read);
                        if(local_length_read>MAX_LENGTH){
                            throw std::runtime_error("reply too large");
                        }
                        //读消息正文
                        char msg[MAX_LENGTH] {0};
                        size_t msg_length=asio::read(sock,asio::buffer(msg,local_length_read));
                        Json::Value root_read;
                        Json::Reader reader;
                        reader.parse(msg,root_read);
                        std::cout<<"reply is:";
                        std::cout<<root_read["data"].asString()<<" "<<root_read["id"].asUInt()<<std::endl;
                        std::cout<<"reply length is:"<<local_length_read<<std::endl;
                        // std::cout<<"data length is :"<<root["data"].toStyledString().length()<<std::endl;
                        // std::cout<<"msg id is:"<<local_msg_id<<std::endl;
                    }
                    }catch (std::exception& e) {
                        std::cout<<"throw exception:"<<e.what()<<std::endl;
                    }
                }
        );
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        }   
        for (auto& t : v_threads) {
            t.join();
        }
        auto end_time=std::chrono::high_resolution_clock::now();
        auto duration=std::chrono::duration_cast<std::chrono::seconds>(end_time-start_time);
        std::cout<<"time cost:"<<duration.count()<<"seconds."<<std::endl;
        getchar();
        return 0;
}
