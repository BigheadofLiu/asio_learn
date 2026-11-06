#include "cserver.hpp"
#include <csignal>
#include <exception>
#include <iostream>
#include <boost/signals2.hpp>
#include "asio_io_context_poll.hpp"

int main(){
    try {
        auto poll=asio_io_context_poll::get_instance();  //获取 io_context池 单例对象
        boost::asio::io_context ioc;   //创建而不是从 poll 中拿 用于主线程 accept和 listen
        // boost::asio::io_context& ioc=poll->get_io_service();
        //添加优雅退出 asio 写法
        boost::asio::signal_set singnals(ioc,SIGINT,SIGTERM);  //设置信号集合
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
