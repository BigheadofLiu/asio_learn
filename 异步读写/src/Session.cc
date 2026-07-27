#include "Session.hpp"
#include <functional>

Session::Session(std::shared_ptr<asio_socket> socket):_send_pending(false),_socket(socket),_recv_pending(false){
    //使用列表初始化
}

void Session::Connect(asio_endpoint& ep){
    _socket->connect(ep);
}

// void Session::WriteCallBackErr(boost::system::error_code& ec,
//     std::size_t bytetransferred,std::shared_ptr<MsgNode> msg_node){
//     if(bytetransferred+msg_node->_cur_length<msg_node->_total_length){
//         msg_node->_cur_length+=bytetransferred;
//         _socket->async_write_some(boost::asio::buffer(_send_node->_msg+_send_node->_cur_length,
//             _send_node->_total_length-_send_node->_cur_length),
//                 std::bind(&Session::WriteCallBackErr,this,std::placeholders::_1,std::placeholders::_2,_send_node));
//     }
// }

void Session::WriteCallBack(boost::system::error_code& ec,
        std::size_t bytetransferred){
            if(ec.value() != 0){
                std::cout<<"error,ec value is:"<<ec.value()<<"massage is:"<<ec.message()<<std::endl;
                return;
            }
            auto& send_node=_send_queue.front();
            send_node->_cur_length+=bytetransferred;
            if(send_node->_cur_length < send_node->_total_length){
                _socket->async_write_some(boost::asio::buffer(send_node->_msg + send_node->_cur_length,
                    send_node->_total_length-send_node->_cur_length),
                        std::bind(&Session::WriteCallBack,this,std::placeholders::_1,std::placeholders::_2));
                    return;
            }
            //发送完毕 队首元素出队
            _send_queue.pop();
            //队列为空则置标志位为 false
            if(_send_queue.empty()){
                _send_pending=false;
            }else
            {   //队列不空则继续发送
                //下个节点开始发送
                auto& send_node=_send_queue.front();
                // send_node->_cur_length+=bytetransferred; //不是完全复制过来
                // if(send_node->_cur_length < send_node->_total_length){  //刚构造的节点_cur_length一定为 0
                _socket->async_write_some(boost::asio::buffer(send_node->_msg + send_node->_cur_length,
                    send_node->_total_length - send_node->_cur_length),
                        std::bind(&Session::WriteCallBack,this,std::placeholders::_1,std::placeholders::_2));
                // }
            }
        }

void Session::WriteALLCallBack(boost::system::error_code& ec,  //使用 asio::async_write() 只需回调一次
        std::size_t bytetransferred){
            if(ec.value()!=0){
                std::cout<<"error,ec value is:"<<ec.value()<<"massage is:"<<ec.message();
                return;
            }
           _send_queue.pop();
           
           if(_send_queue.empty()){
                _send_pending=false;
           }else{
                auto& send_node=_send_queue.front();
                boost::asio::async_write(*_socket,
                    boost::asio::buffer(send_node->_msg+send_node->_cur_length,send_node->_total_length-send_node->_cur_length),
                      bind(&Session::WriteALLCallBack,this,std::placeholders::_1,std::placeholders::_2));
           }
        }

// void Session::WriteToSocketErr(const std::string& buf){
//     this->_send_node=std::make_shared<MsgNode>(buf.c_str(),buf.length());
//     _socket->async_write_some(boost::asio::buffer(_send_node->_msg,_send_node->_total_length),
//         std::bind(&Session::WriteCallBackErr,this,std::placeholders::_1,std::placeholders::_2,_send_node));
// }

void Session::WriteToSocket(const std::string& buf){
    this->_send_queue.emplace(new MsgNode(buf.c_str(),buf.length()));
    if(_send_pending){
        return;
    } 
    //异步不会一次性发完
    _socket->async_write_some(boost::asio::buffer(buf),
        std::bind(&Session::WriteCallBack,this,std::placeholders::_1,std::placeholders::_2));
    //标志位设置为:true
    _send_pending=true;
}
//使用 asio::async_write() 只需一次回调
void Session::WriteALlToSocket(const std::string& buf){
    _send_queue.emplace(new MsgNode(buf.c_str(),buf.length()));
    if(_send_pending){
        return;
    }
    boost::asio::async_write(*_socket,boost::asio::buffer(buf),
        std::bind(&Session::WriteALLCallBack,this,std::placeholders::_1,std::placeholders::_2));
    // _socket->async_send(boost::asio::buffer(buf),std::bind(&Session::WriteALLCallBack,this,std::placeholders::_1,std::placeholders::_2)); //教程里写的不对吧。。。
    _send_pending=true;
}

//读相关
void  Session::ReadFromSocket(){
    if(_recv_pending){
        return;
    }
    _recv_node=std::make_shared<MsgNode>(RECVSIZE);
    _socket->async_read_some(boost::asio::buffer(_recv_node->_msg,_recv_node->_total_length),
        std::bind(&Session::ReadCallBack,this,std::placeholders::_1,std::placeholders::_2));
    _recv_pending=true;
}
void Session::ReadCallBack(boost::system::error_code& ec,size_t bytetransferred){
    if(ec.value()!=0){
        std::cout<<"error!,ec value is:"<<ec.value()<<"ec massage is:"<<ec.message()<<std::endl;
        return;
    }
    _recv_node->_cur_length += bytetransferred;
    if(_recv_node->_cur_length < _recv_node->_total_length){
        _socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_length,
            _recv_node->_total_length-_recv_node->_cur_length),
                std::bind(&Session::ReadCallBack,this,std::placeholders::_1,std::placeholders::_2));
        return;
    }
    _recv_pending=false;
    _recv_node=nullptr;
}

void Session::ReadALlFromSocket(){
    if(_recv_pending){
        return;
    }
    _recv_node=std::make_shared<MsgNode>(RECVSIZE);
    asio::async_read(*_socket,asio::buffer(_recv_node->_msg,_recv_node->_total_length),
    std::bind(&Session::ReadAllCallBack,this,std::placeholders::_1,std::placeholders::_2));
    _recv_pending=true;
}
void Session::ReadAllCallBack(boost::system::error_code& ec,std::size_t bytetransferred){
    if(ec.value()!=0){
        std::cout<<"error!,ec value is:"<<ec.value()<<"ec massage is:"<<ec.message()<<std::endl;
        return;
    }
    _recv_node->_cur_length += bytetransferred;
    //读取数据交给子线程处理
    _recv_pending=false;
    _recv_node=nullptr;
    
    ReadAllCallBack;
}