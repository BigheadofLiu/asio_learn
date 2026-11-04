#include "cserver.hpp"
#include <csignal>
#include <exception>
#include <iostream>
#include <boost/signals2.hpp>

int main(){
    try {
        boost::asio::io_context ioc;
        //添加优雅退出
        boost::asio::signal_set singnals(ioc,SIGINT,SIGTERM);
        singnals.async_wait([&ioc](auto arg_1,auto arg_2){
            // std::cout<<"stop signal ec_code is:"<<arg_1<<" "<<"stop signal code is:"<<arg_2<<std::endl;
            ioc.stop();
        });
        cserver server(ioc,8899);
        ioc.run();
    } catch (std::exception& e) {
        std::cerr<<"throw exception:"<<e.what()<<std::endl;
    }
}
