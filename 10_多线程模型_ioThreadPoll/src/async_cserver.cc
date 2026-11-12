#include "cserver.hpp"
#include <csignal>
#include <exception>
#include <iostream>
#include <boost/signals2.hpp>
#include "asio_thread_poll.hpp"

int main(){
    try {
        auto& ioc=asio_thread_poll::get_instance()->get_ioc_context();
        //添加优雅退出 asio 写法
        boost::asio::signal_set singnals(ioc,SIGINT,SIGTERM);  //设置信号集合
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
