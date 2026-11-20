#include <boost/asio.hpp>
#include <csignal>
#include <exception>
#include <iostream>
#include "server.hpp"
#include "io_context_poll.hpp"

int main(){
    try {
        auto poll=io_context_poll::get_instance();
        boost::asio::io_context ioc;
        boost::asio::signal_set singnals(ioc,SIGINT,SIGTERM);
        singnals.async_wait([&ioc,poll](auto args_1,auto args_2){
            ioc.stop();
            poll->stop();
        });
        server server(ioc,8899);
        ioc.run();
    } catch (const std::exception& e) {
        std::cout<<e.what()<<std::endl;
    }
}


