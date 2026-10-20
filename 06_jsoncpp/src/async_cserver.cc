#include "cserver.hpp"
#include <exception>
#include <iostream>

int main(){
    try {
        boost::asio::io_context ioc;
        cserver server(ioc,8899);
        ioc.run();

    } catch (std::exception& e) {
        std::cerr<<"throw exception:"<<e.what()<<std::endl;
    }
}
