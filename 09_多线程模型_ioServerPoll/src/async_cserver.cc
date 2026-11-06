#include "cserver.hpp"
#include <csignal>
#include <exception>
#include <iostream>
#include <boost/signals2.hpp>
#include "asio_io_context_poll.hpp"

int main(){
    try {
        auto poll=asio_io_context_poll::get_instance();
        boost::asio::io_context ioc;
        //添加优雅退出 asio 写法
        boost::asio::signal_set singnals(ioc,SIGINT,SIGTERM);
        singnals.async_wait([&ioc,poll](auto arg_1,auto arg_2){
            // std::cout<<"stop signal ec_code is:"<<arg_1<<" "<<"stop signal code is:"<<arg_2<<std::endl;
            ioc.stop();
            poll->stop();
        });
        cserver server(ioc,8899);
        ioc.run();
    } catch (std::exception& e) {
        std::cerr<<"throw exception:"<<e.what()<<std::endl;
    }
}
