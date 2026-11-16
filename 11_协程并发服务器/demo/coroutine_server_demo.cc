#include <boost/asio.hpp>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>
#include <ostream>
#include <termios.h>
#include <vector>

//协程关键字
// co_await;
// co_return;
// co_yield;

boost::asio::awaitable<void> echo(boost::asio::ip::tcp::socket socket){
    try {
        // char data[1024];
        std::vector<char> data(1024);
        for(;;){
            auto size=co_await socket.async_read_some(
                boost::asio::buffer(data),
                boost::asio::use_awaitable
            );
            co_await boost::asio::async_write(
                socket,
                boost::asio::buffer(data,size),
                boost::asio::use_awaitable
            );
        }
    } catch (const boost::system::system_error& e) {
        if(e.code()==boost::asio::error::eof){
            std::cout<<"client disconnected"<<std::endl;
        }else {
            std::cout<<"throw error:"<<e.what()<<std::endl;
        }
    }
}   

boost::asio::awaitable<void> listener(){
    auto executor=co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor(executor,{boost::asio::ip::tcp::v4(),8899});
    for(;;){
        // boost::asio::ip::tcp::socket 
        auto socket=co_await acceptor.async_accept(boost::asio::use_awaitable);
        boost::asio::co_spawn(executor,echo(std::move(socket)),boost::asio::detached);
    }
}

int main(){
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signal(ioc,SIGINT,SIGTERM);
        signal.async_wait([&ioc](auto args1,auto args2){
            ioc.stop();
        });
        boost::asio::co_spawn(ioc,listener(),boost::asio::detached);
        ioc.run();
    } catch (std::exception& e) {
        std::cout<<"throw error:"<<e.what()<<std::endl;
    }
    return 0;
}