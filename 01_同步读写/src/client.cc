#include "client.hpp"
#include <vector>

namespace ip=boost::asio::ip;
namespace asio=boost::asio;

int main(){
    asio::io_context ioc;
    ip::tcp::socket sock(ioc);

    std::string raw_ip_addr="127.0.0.1";
    unsigned short port=8899;
    ip::address ip_addr=ip::make_address(raw_ip_addr); //物理 ip->asio ip
    ip::tcp::endpoint server_ep(ip_addr,port);  //构造通信端点

   
    std::cout<<"正在连接到服务端...\n";
    sock.connect(server_ep);
    std::cout<<"连接成功！\n";

    //通信
    //收
    std::vector<char> buf(128);  //数据缓冲区
    boost::system::error_code ec;
    size_t read_len= sock.read_some(asio::buffer(buf),ec);
    if(!ec){
        std::cout<<"收到服务端消息："<<std::endl;
        std::cout.write(buf.data(),read_len);
        std::cout<<std::endl;   
    }
    //发
    std::string msg {};
    std::cout<<"客户端说：";
    std::cin>>msg;
    // size_t write_len=sock.write_some(asio::buffer(msg),ec);  error write 必须用全局 write
    size_t write_len=asio::write(sock,asio::buffer(msg),ec);
    if(!ec){
        std::cout<<"发送成功！"<<std::endl;
    }
    return 0;
}
