#include "server.hpp"
#include <set> //using set
#include <memory> //using share_ptr
namespace asio=boost::asio;
namespace ip=boost::asio::ip;
const int MAX_LENGTH=1024;
using socket_prt=std::shared_ptr<ip::tcp::socket>;
std::set<std::shared_ptr<std::thread>> thread_set;

void session(socket_prt sp){
    try
    {
        for(;;){
            char data[MAX_LENGTH]={};
            boost::system::error_code ec;
            // size_t length=asio::read(sp,asio::buffer(data,MAX_LENGTH),ec);
            size_t length=sp->read_some(asio::buffer(data,MAX_LENGTH),ec);

            if(ec==asio::error::eof){
                std::cout<<"connect closed by peer"<<std::endl;
                break;
            }else if (ec)
            {
                throw boost::system::system_error(ec);
            }
            std::cout<<"receive from:"<<sp->remote_endpoint().address().to_string()<<std::endl;
            std::cout<<"message is:"<<data<<std::endl;
            
            //写回
            asio::write(*sp,asio::buffer(data,length));  
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}
void server(asio::io_context& ioc,unsigned short port){
    ip::tcp::acceptor acc(ioc,ip::tcp::endpoint(ip::tcp::v4(),port));
    for(;;){
        socket_prt sp(new ip::tcp::socket(ioc));
        acc.accept(*sp);
        // session(sp);  //避免使用主线程 会导致阻塞
        auto t=std::make_shared<std::thread>(session,sp); //使用子线程
        thread_set.insert(t); //加入集合中 防止局部变量失效
    }
}
int main(){
    try
    {
        asio::io_context ioc;
        server(ioc,8899);
         for(auto& i:thread_set){
            i->join();
         }
    } 
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}