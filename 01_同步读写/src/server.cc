#include "server.hpp"
namespace ip=boost::asio::ip;
namespace asio=boost::asio;

int main(){
    asio::io_context ioc;
    ip::tcp::socket sock(ioc);

    // unsigned short port=8899; //端口号
    // ip::address addr=ip::address_v4::any(); //允许连接的客户端 ip 类型
    // ip::tcp::endpoint client_ep(addr,port); //构造通信端点
    ip::tcp::endpoint client_ep(asio::ip::tcp::v4(),8899);
    ip::tcp::acceptor acceptor(ioc,client_ep); //新版 boost 写法 自动 执行 open bind listen
    std::cout<<"服务器已启动，等待客户端连接..."<<std::endl;

    acceptor.accept(sock);
    std::cout<<"客户端已连接！"<<std::endl;
    //打印连接客户端信息
    std::cout<<"客户端ip:"<<sock.remote_endpoint().address().to_string()<<std::endl;
    //通信
    // std::string msg="hello from server!\n";
    std::string msg {};
    std::cout<<"服务器说：";
    std::cin>>msg;
    boost::system::error_code ec;
    asio::write(sock,asio::buffer(msg),ec);
    std::cout<<"消息已发送！\n";

    std::vector<char> buf(128);
    size_t read_len=sock.read_some(asio::buffer(buf),ec);
    if(!ec){
        std::cout<<"收到客户端消息:"<<std::endl;
        std::cout.write(buf.data(),read_len)<<std::endl;
    }
    return 0;
}
