#include <cstddef>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

class Session : public std::enable_shared_from_this<Session> {  //继承enable_shared_from_this 使用share_from_this 延长 shared在回调函数 智能指针生命周期
    public:
    Session(boost::asio::io_context &ioc):_socket(ioc){

    }
    boost::asio::ip::tcp::socket& get_socket(){
        return _socket;
    }
    void start();
    ~Session();
    private:
    boost::asio::ip::tcp::socket _socket; //工作 socket
    enum{max_length=1024}; 
    char _data[max_length];  //接收数据
    void handle_write(const boost::system::error_code& ec/*,size_t bytetransferred*/); //不考虑粘包的情况 写回调
    void handle_read(const boost::system::error_code& ec,size_t byte_transferred); //读回调

    
};
