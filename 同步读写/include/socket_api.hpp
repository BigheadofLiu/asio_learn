#pragma once
#include <iostream>
#include <boost/asio.hpp>

extern int create_tcp_socket();         // 创建用于通信的tcp socket    
extern int create_acceptor_socket();    // 创建用于accept的socket 用于 listen
extern int bind_acceptor_socket();      // 服务端绑定端点 bind
extern int accept_new_connection();     // 服务端接收新连接 accept
extern int connect_to_end();            // 客户端连接到端点 connect
extern int connect_to_dns_end();        //连接到域名
extern void use_const_buffer();
extern void use_buffer_str();
extern void use_buffer_arry();
extern void write_to_socket(asio::ip::tcp::socket&);  //use write_some
extern void send_data_by_write_to_socket(); //by read_some
extern int send_data_by_send(); //use send
extern int send_data_by_write(); //use write //asio::write
extern std::string read_from_socket(asio::ip::tcp::socket&); //read_some
extern int read_data_by_read_from_socket();