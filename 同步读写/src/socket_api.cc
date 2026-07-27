#include "socket_api.hpp"

namespace asio = boost::asio;

//创建用于通信的tcp socket（用于通讯的套接字）
int create_tcp_socket()
{  
    //boost 旧版本
    // asio::io_context ioc;  //ioc用于与操作系统交换信息 屏蔽底层
    // asio::ip::tcp::socket sock(ioc); //asio socket 类绑定 ioc
    // asio::ip::tcp protocol=asio::ip::tcp::v4();  
    // boost::system::error_code ec;
    // sock.open(protocol,ec);
    // if(ec.value()!=0)
    // {
    //     std::cout<<"socket打开失败，错误码："<<ec.value()
    //     <<"，错误信息："<<ec.message()<<std::endl;
    //     return ec.value();
    // }

    //对照 socket
    // int cfd=socket(AF_INET,SOCK_STREAM,0);
    // if(cfd==-1){
    //     perror("socket");
    // }

    //boost新版本
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc,asio::ip::tcp::v4());
    return 0;
}

//创建用于 accept 的 socket（用于监听的套接字）
int create_acceptor_socket()
{   
    //老版本
    // asio::io_context ioc;
    // asio::ip::tcp::acceptor acceptor(ioc);
    // asio::ip::tcp protocol=asio::ip::tcp::v4();
    // boost::system::error_code ec;
    // acceptor.open(protocol,ec);
    // if(ec.value()!=0)
    // {
    //     std::cout<<"acceptor socket打开失败，错误码："<<ec.value()
    //     <<"，错误信息："<<ec.message()<<std::endl;
    //     return ec.value();
    // }
    
    //原生 C Socket 对比：相当于底层调用了 socket()
    // int lfd = socket(AF_INET, SOCK_STREAM, 0);
    // if(lfd == -1) {
    //     perror("socket");
    // }
    
    //Boost.Asio 一步到位法
    // 这一行代码，等价于原生 C Socket 连续调用了：
    // 1. socket()
    // 2. bind(INADDR_ANY, 8899)
    // 3. listen()
    asio::io_context ioc;
    asio::ip::tcp::acceptor accept(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 8899));

    return 0;
}

// 服务端绑定端点
int bind_acceptor_socket() {

    asio::io_context ioc;
    unsigned short port_num = 8899;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    // 构造 Acceptor 并打开协议
    asio::ip::tcp::acceptor acceptor(ioc, ep.protocol());
    
    boost::system::error_code ec;
    // 绑定端点
    acceptor.bind(ep, ec);
    if(ec.value() != 0) {
        std::cout << "绑定失败，错误码：" << ec.value() 
                  << "，错误信息：" << ec.message() << std::endl;
        return ec.value();
    }
    std::cout << "Acceptor 绑定成功！" << std::endl;
    return 0;

    //新版此步骤省略 自动执行可 open bind listen
}

// 客户端连接到端点
int connect_to_end() {
    // boost老版本
    // asio::io_context ioc;
    // std::string raw_ip_addr = "127.0.0.1";
    // unsigned short port_num = 8899;
    // boost::system::error_code ec;
    
    // asio::ip::address ip_address = asio::ip::make_address(raw_ip_addr, ec);
    // asio::ip::tcp::endpoint ep(ip_address, port_num);
    // asio::ip::tcp::socket sock(ioc,ep.protocol());
    // // 连接到服务端
    // sock.connect(ep, ec);
    // if(ec.value() != 0) {
    //     std::cout << "连接失败，错误码：" << ec.value() 
    //               << "，错误信息：" << ec.message() << std::endl;
    //     return ec.value();
    // }
    // std::cout << "成功连接到服务端！" << std::endl;
    // return 0;

    //boost 新版本
    asio::io_context ioc;
    asio::ip::tcp::endpoint ep(asio::ip::make_address_v4("127.0.0.1"),8899);
    asio::ip::tcp::socket sock(ioc);
    sock.connect(ep);
}

// 服务端接收新连接
int accept_new_connection() {
    asio::io_context ioc;
    unsigned short port_num = 8899;
    asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port_num);
    
    //老版本
    asio::ip::tcp::acceptor acceptor(ioc,ep.protocol());
    acceptor.bind(ep);
    boost::system::error_code ec;
    // 开始监听
    acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if(ec.value() != 0) {
        std::cout << "监听失败，错误码：" << ec.value() << std::endl;
        return ec.value();
    }
    // 创建一个新的 socket 用于和客户端通信
    asio::ip::tcp::socket sock(ioc);
    std::cout << "服务端正在等待客户端连接..." << std::endl;
    
    // 阻塞等待连接
    acceptor.accept(sock, ec);
    if(ec.value() != 0) {
        std::cout << "接收连接失败，错误码：" << ec.value() << std::endl;
        return ec.value();
    }
    std::cout << "成功接收到一个新连接！客户端IP：" 
              << sock.remote_endpoint().address().to_string() << std::endl;

    //新版本
    // 直接传入 ep 进行构造，会自动执行 open 和 bind listen
    // asio::ip::tcp::acceptor acceptor(ioc, ep);
    return 0;
}

int connect_to_dns_end(){
    //连接到域名
    std::string host="www.demo.com";
    std::string port="8899";
    asio::io_context ioc;
    asio::ip::tcp::resolver res(ioc);
    boost::system::error_code ec;
    auto result=res.resolve(host,port,ec);
    if(ec){
        std::cout<<"域名解析失败:"<<ec.message()<<std::endl;
        return ec.value();
    }
    asio::ip::tcp::socket sock(ioc);
    asio::connect(sock,result,ec);   //使用全局连接
    if(ec){
        std::cout<<"服务器连接失败！"<<std::endl;
        return ec.value();
    }
    std::cout<<"成功连接到域名！"<<std::endl;
    return 0;
}

/**
 * buffer相关
 * */
void use_const_buffer(){
    //最原始做法
    std::string msg="hello world";
    asio::const_buffer asio_buffer(msg.c_str(),msg.length());
    std::vector<asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buffer);
    
    //然后就可以直接发送了 仅做演示
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc);
    sock.send(buffers_sequence);
}
void use_buffer_str(){
    std::string msg="hello world";
    auto out_put=asio::buffer(msg);

    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc);
    sock.send(out_put);
}
void use_buffer_arry(){
    //数组
    const size_t ARRY_SIZE=20;
    std::unique_ptr<char[]> buf(new char[ARRY_SIZE]);
    auto input_buf=asio::buffer(static_cast<void*>(buf.get()),ARRY_SIZE);

    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc);
    sock.send(input_buf);
}

//同步写 write_some sock.write_some
void write_to_socket(asio::ip::tcp::socket& sock){
    std::string str {"hello world!"};
    size_t total_bytes_writen {0};
    while(total_bytes_writen!=str.length()){
        //write_some（内存地址，数据长度）
        //wrtie_some 缓冲区满则不一次性写完 返回写入的字节数
        //use write_some
        total_bytes_writen+=sock.write_some(asio::buffer(str.c_str()+total_bytes_writen,str.length()-total_bytes_writen));
    }
}
//send_data_by_write_some
void send_data_by_write_to_socket(){
    asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"),8899);
    asio::io_context ioc;
    asio::ip::tcp::socket sock(ioc,ep.protocol());
    sock.connect(ep);
    write_to_socket(sock);
}

//同步写 send  sock.send
//send_data_by_send
//send一次性发完 发不完则阻塞 直至发送完成
int send_data_by_send(){
    try
    {
        asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"),8899);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc,ep.protocol());
        sock.connect(ep);

        std::string buf="hello world!";
        int send_length=sock.send(asio::buffer(buf.c_str(),buf.length()));  //use sock.send()
        if(!send_length){
            std::cout<<"send failed！"<<std::endl;
            return 0;
        }
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"error code:"<<e.code()
        <<"message:"<<e.what();
        return e.code().value();
    }
    return 0;
}
//同步写 asio::write 
int send_data_by_write(){
     try
    {
        asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"),8899);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc,ep.protocol());
        sock.connect(ep);

        std::string buf="hello world!";
        int send_length=asio::write(sock,asio::buffer(buf.c_str(),buf.length()));  //use asio::write()
        if(!send_length){
            std::cout<<"write failed！"<<std::endl;
            return 0;
        }
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"error code:"<<e.code()
        <<"message:"<<e.what();
        return e.code().value();
    }
    return 0;
}

//同步读 read_some  可不一次性读完 返回读入的字节数 sock.read_some
std::string read_from_socket(asio::ip::tcp::socket& sock){
    const unsigned char MASSAGE_SIZE {7};
    char buf[MASSAGE_SIZE] {};
    size_t total_bytes_read {0};
    while(total_bytes_read!=MASSAGE_SIZE){
        //read_some（内存地址，数据长度）
        //read_some 缓冲区满则不一次性写完 返回写入的字节数
        //use read_some
        total_bytes_read+=sock.read_some(asio::buffer(buf+total_bytes_read,MASSAGE_SIZE-total_bytes_read));
    }
    return std::string(buf,total_bytes_read);
}
int read_data_by_read_from_socket(){
    try
    {
        asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"),8899);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc,ep.protocol());
        sock.connect(ep);
        read_from_socket(sock);
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"error code:"<<e.code()
        <<"message:"<<e.what();
        return e.code().value();
    }
    return 0;
}

//总结
//write_some socket.write_some()
//send  socket.send()
//write  asio::write()

//read_some  socket.read_some()
//receive socket.receive()
//read  asio::read()



