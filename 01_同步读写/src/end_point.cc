#include "end_point.hpp"


// 可以使用别名简化代码
namespace asio = boost::asio;
//创建客户端终端通信节点
int client_end_point()
{   //step1 指定本地ip 和端口号
    std::string raw_ip_addr="127.0.0.1";   //地址
    unsigned short port_num=8899;  //端口号
    boost::system::error_code ec;   //错误码

    //step2 绑定asio ip地址类
    asio::ip::address ip_address=asio::ip::make_address(raw_ip_addr,ec);  //创建地址
    // asio::ip::address ip_address=asio::ip::from_string() 废弃
    //错误码判断
    if(ec.value()!=0)
    {
        std::cout<<"ip_address解析错误，错误码："<<ec.value()
        <<"，错误信息："<<ec.message()<<std::endl;
        return ec.value();
    }
    //step3 tcp通信节点绑定 asio 地址类和本地端口号
    asio::ip::tcp::endpoint client_ep(ip_address,port_num);  //传入服务器IP和端口 客户端point需要填入目标服务器 ip 和 port
    return 0;
}
//创建服务器通信终端节点
int server_end_point()
{
    unsigned short port_num=8899;  //服务端口
    // asio::ip::address ip_address=asio::ip::address_v6::any(); //指定可连接客户端类型（v4 or v6）
    asio::ip::address ip_address=asio::ip::address_v4::any();
    asio::ip::tcp::endpoint server_ep(ip_address,port_num); //传入本地 ip 和端口号
    return 0;
}


 


