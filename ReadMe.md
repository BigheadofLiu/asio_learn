# Boost.Asio 网络编程学习项目

这是一个使用 C++17 和 Boost.Asio 编写的 TCP 网络编程练习项目。项目按照学习难度分为三个部分：

```text
01_同步读写          学习 TCP 和同步 socket
        ↓
02_异步读写          学习异步读写、消息节点和发送队列
        ↓
03_异步读写_demo     组合成一个可以运行的异步 Echo Server
```

## 环境

- C++17
- CMake 3.16+
- Boost.Asio
- Boost.System
- macOS 或 Linux

## 构建

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build --parallel
```

重新构建：

```bash
rm -rf build
cmake -S . -B build
cmake --build build --parallel
```

## 01：同步读写

目录：

```text
01_同步读写/
├── include/
└── src/
    ├── client.cc
    ├── client_sync.cc
    ├── end_point.cc
    ├── server.cc
    ├── server_sync.cc
    └── socket_api.cc
```

### 这一部分做什么

`01` 用同步方式完成 TCP 客户端和服务端通信，重点是理解最基础的网络流程：

```text
创建 socket
    ↓
创建 endpoint
    ↓
服务端 bind、listen、accept
    ↓
客户端 connect
    ↓
read / write
```

### 基础客户端和服务端

服务端：

```bash
./build/sync_server
```

客户端：

```bash
./build/sync_client
```

相关文件：

- `01_同步读写/src/server.cc`
- `01_同步读写/src/client.cc`

### 多线程 Echo Server

服务端：

```bash
./build/sync_echo_server
```

客户端：

```bash
./build/sync_echo_client
```

这个示例中，主线程负责 `accept`，每接收一个客户端，就创建一个子线程处理该客户端的读写：

```text
主线程 accept
    ├── 子线程处理客户端 A
    ├── 子线程处理客户端 B
    └── 子线程处理客户端 C
```

相关文件：

- `01_同步读写/src/server_sync.cc`
- `01_同步读写/src/client_sync.cc`

### 这一部分要理解的内容

- `socket`、`endpoint`、`acceptor`
- `bind`、`listen`、`accept`、`connect`
- `read_some` 和 `write_some`
- `asio::read` 和 `asio::write`
- 阻塞 I/O
- 使用线程处理多个客户端
- TCP 是字节流，没有消息边界

## 02：异步读写

目录：

```text
02_异步读写/
├── include/
│   ├── MsgNode.hpp
│   └── Session.hpp
└── src/
    ├── MsgNode.cc
    └── Session.cc
```

### 这一部分做什么

`02` 是异步读写组件练习，没有 `main()`，不能直接运行。它主要研究：

- 异步操作如何处理部分读写
- 如何保存异步操作使用的数据
- 如何使用发送队列保证消息顺序
- 如何通过回调继续下一次读写

CMake 会把它编译为：

```text
build/libasync_session.a
```

### `MsgNode`

[MsgNode](file:///Users/imyourfather1/Desktop/Demo/Cpp/asio_learn/02_异步读写/include/MsgNode.hpp) 用来保存一条消息：

```cpp
int _total_length; // 总长度
int _cur_length;   // 已经处理的长度
char* _msg;        // 数据缓冲区
```

异步操作启动后，函数可能立即返回，因此 buffer 指向的内存必须继续存在。`MsgNode` 负责保存这块内存和当前读写进度。

### `Session`

[Session](file:///Users/imyourfather1/Desktop/Demo/Cpp/asio_learn/02_异步读写/include/Session.hpp) 管理一个 socket，并保存：

```cpp
std::queue<std::shared_ptr<MsgNode>> _send_queue;
std::shared_ptr<MsgNode> _recv_node;
```

发送流程：

```text
消息进入发送队列
    ↓
async_write_some 发送一部分
    ↓
回调更新 _cur_length
    ↓
还有剩余数据则继续发送
    ↓
当前消息完成后发送下一条
```

`async_read_some` 和 `async_write_some` 可能只完成部分操作，因此需要自己记录 `bytetransferred` 并继续读写。`asio::async_read` 和 `asio::async_write` 则会在内部处理部分读写，直到满足完整传输条件。

## 03：异步读写 Demo

目录：

```text
03_异步读写_demo/
├── include/
│   ├── Client.hpp
│   ├── Server.hpp
│   └── Session.hpp
└── src/
    ├── async_server.cc
    ├── Client.cc
    ├── Server.cc
    └── Session.cc
```

### 这一部分做什么

`03` 把异步服务器的完整流程组合起来，实现一个异步 Echo Server：

```text
客户端发送消息
        ↓
Session 异步读取
        ↓
服务端输出消息
        ↓
Session 异步写回
        ↓
客户端收到原消息
```

### 运行

先启动服务端：

```bash
./build/async_server
```

再打开另一个终端启动客户端：

```bash
./build/async_client
```

服务端监听 `127.0.0.1:8899`。当前客户端发送一条消息并收到回复后就会退出；因此服务端随后打印 `End of file`，表示客户端已经正常关闭连接。

### 代码关系

```text
async_server.cc
    ↓
Server
    ├── acceptor 监听端口
    └── 接受连接并创建 Session
            ↓
        Session
            ├── socket
            ├── async_read_some
            ├── handle_read
            ├── async_write
            └── handle_write
```

`Server` 负责监听和接受连接，`Session` 负责一个客户端连接的全部读写。每个客户端对应一个独立的 `Session`。

### `io_context`

[async_server.cc](file:///Users/imyourfather1/Desktop/Demo/Cpp/asio_learn/03_异步读写_demo/src/async_server.cc) 中：

```cpp
boost::asio::io_context ioc;
Server server(ioc, 8899);
ioc.run();
```

`Server` 负责注册异步操作，`io_context.run()` 负责驱动这些操作并执行回调。异步操作不会自动创建线程；当前 Demo 中，回调默认运行在调用 `run()` 的线程中。

### `Session` 的生命周期

`Session` 继承：

```cpp
std::enable_shared_from_this<Session>
```

并通过：

```cpp
auto session = std::make_shared<Session>(ioc);
```

创建对象。异步回调中使用：

```cpp
auto self = shared_from_this();
```

让回调保存 `shared_ptr<Session>`，保证异步操作完成之前 `Session` 不会被提前销毁。

## 三个部分的区别

| 部分 | 主要内容 | 是否可以直接运行 |
|---|---|---|
| `01_同步读写` | 同步 socket、线程和基础 API | 可以 |
| `02_异步读写` | `MsgNode`、异步读写和发送队列 | 不可以，组件库 |
| `03_异步读写_demo` | `Server`、`Session`、异步 Echo Server | 可以 |

## 当前 Demo 的限制

- TCP 只提供字节流，不保证一次读操作对应一次消息。
- `03` 使用固定大小的字符数组，主要用于学习异步流程。
- 真实项目需要设计消息协议，例如长度字段、固定长度或分隔符。
- `02` 是早期组件练习，代码重点是展示思路，不是生产级网络库。
- 客户端断开时服务端收到 `boost::asio::error::eof`，这是正常的对端关闭事件。

## 推荐学习顺序

1. 阅读 `01_同步读写/src/server.cc` 和 `client.cc`，理解 TCP 基本流程。
2. 阅读 `01_同步读写/src/server_sync.cc`，理解线程处理多个客户端。
3. 阅读 `02_异步读写/include/MsgNode.hpp`，理解异步 buffer 的生命周期。
4. 阅读 `02_异步读写/src/Session.cc`，理解部分读写和发送队列。
5. 阅读 `03_异步读写_demo/src/Server.cc`，理解异步 accept。
6. 阅读 `03_异步读写_demo/src/Session.cc`，理解异步读写回调。
7. 阅读 `async_server.cc`，理解 `io_context.run()` 如何驱动整个程序。
