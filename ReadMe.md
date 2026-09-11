# Boost.Asio 读写学习项目

这是一个基于 C++ 和 Boost.Asio 的 TCP 通信学习项目，主要通过同步读写和异步读写两部分，练习 socket、endpoint、acceptor、buffer、读写操作以及异步回调。

## 学习内容

- TCP 服务端和客户端的基本通信流程
- `io_context`、`socket`、`acceptor` 和 `endpoint` 的使用
- 同步读写和异步读写的区别
- `read_some`、`write_some`、`send`、`receive`、`asio::read` 和 `asio::write`
- `asio::buffer` 对字符串、数组和连续内存的封装
- 使用线程处理多个同步客户端连接
- 使用消息节点和队列管理异步发送数据

## 项目结构

```text
.
├── CMakeLists.txt
├── 同步读写/
│   ├── include/
│   │   ├── client.hpp
│   │   ├── end_point.hpp
│   │   ├── server.hpp
│   │   └── socket_api.hpp
│   └── src/
│       ├── client.cc
│       ├── client_sync.cc
│       ├── end_point.cc
│       ├── server.cc
│       ├── server_sync.cc
│       └── socket_api.cc
└── 异步读写/
    ├── include/
    │   ├── MsgNode.hpp
    │   └── Session.hpp
    └── src/
        ├── MsgNode.cc
        └── Session.cc
```

## 环境依赖

- CMake 3.16 或更高版本
- C++17 编译器
- Boost，包含 Boost.Asio 和 Boost.System
- macOS、Linux 或其他支持 C++17 和 Boost.Asio 的系统

Boost.Asio 在本项目中使用 Boost 版本：

```cpp
#include <boost/asio.hpp>
```

## 构建项目

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

清理构建目录后重新构建：

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

## CMake targets

| Target | 类型 | 说明 |
|---|---|---|
| `sync_server` | 可执行文件 | 单客户端同步服务端 |
| `sync_client` | 可执行文件 | 与 `sync_server` 配合使用的同步客户端 |
| `sync_echo_server` | 可执行文件 | 使用线程处理客户端的同步 Echo 服务端 |
| `sync_echo_client` | 可执行文件 | 与 `sync_echo_server` 配合使用的 Echo 客户端 |
| `sync_socket_api` | 静态库 | 同步 socket、endpoint、buffer 和读写 API 示例 |
| `async_session` | 静态库 | 异步 Session 和消息节点实现 |

同步目录中有多个 `main` 函数，因此每个示例被配置成独立的可执行文件，不能把所有 `.cc` 文件直接编译到同一个 target 中。

## 同步读写

### 基础客户端/服务端

启动服务端：

```bash
./build/sync_server
```

再打开另一个终端启动客户端：

```bash
./build/sync_client
```

这组示例使用一次连接完成基本的收发：

1. 服务端监听 `8899` 端口。
2. 客户端连接 `127.0.0.1:8899`。
3. 服务端读取控制台输入并发送消息。
4. 客户端读取服务端消息，再输入消息发送回服务端。

相关代码：

- 服务端入口：`同步读写/src/server.cc`
- 客户端入口：`同步读写/src/client.cc`

### 多线程 Echo 服务

启动服务端：

```bash
./build/sync_echo_server
```

再打开另一个终端启动客户端：

```bash
./build/sync_echo_client
```

`sync_echo_server` 的工作流程：

1. `acceptor` 监听 `8899` 端口。
2. 接收客户端连接。
3. 为每个客户端创建一个线程。
4. 在线程中调用 `socket::read_some` 接收数据。
5. 使用 `asio::write` 将数据原样写回客户端。

相关代码：

- Echo 服务端：`同步读写/src/server_sync.cc`
- Echo 客户端：`同步读写/src/client_sync.cc`

## 同步读写 API

同步 API 示例位于 `同步读写/src/socket_api.cc`，主要包括：

| API | 特点 |
|---|---|
| `socket.write_some` | 尝试写入一部分数据，返回实际写入字节数 |
| `socket.send` | 同步发送数据 |
| `asio::write` | 持续写入，直到缓冲区数据全部完成或发生错误 |
| `socket.read_some` | 尝试读取一部分数据，返回实际读取字节数 |
| `socket.receive` | 同步接收数据 |
| `asio::read` | 持续读取，直到满足读取条件或发生错误 |

`read_some` 和 `write_some` 不保证一次完成全部数据。因此，当协议要求完整传输固定长度数据时，需要根据返回的字节数循环处理，或者使用 `asio::read` 和 `asio::write`。

## 异步读写

异步部分目前由 `async_session` 静态库提供，核心类是 `Session`：

- `Session` 持有一个 TCP socket。
- `WriteToSocket` 使用 `async_write_some`，需要在回调中继续处理剩余数据。
- `WriteALlToSocket` 使用 `asio::async_write`，由 Asio 持续完成整个缓冲区的写入。
- `_send_queue` 用于保证多个异步发送请求按顺序执行。
- `ReadFromSocket` 使用 `async_read_some`。
- `ReadALlFromSocket` 使用 `asio::async_read`，尝试读取指定长度的数据。
- `MsgNode` 负责保存待发送或待接收的数据、总长度和当前处理位置。

异步读写的基本调用关系如下：

```text
调用异步读写函数
        |
        v
注册 handler
        |
        v
io_context.run()
        |
        v
操作系统完成 I/O
        |
        v
触发回调函数
        |
        v
处理剩余数据或继续下一条消息
```

当前异步目录还没有独立的 `main` 函数，因此它被编译成库，暂时不能直接运行。要形成完整的异步客户端或服务端，还需要补充：

1. 创建 `io_context`。
2. 创建并连接 `Session`。
3. 发起 `ReadFromSocket` 或 `WriteToSocket`。
4. 调用 `io_context.run()` 驱动异步回调。
5. 在实际协议中处理消息边界、连接关闭和错误状态。

## 同步与异步对比

| 对比项 | 同步读写 | 异步读写 |
|---|---|---|
| 调用方式 | 调用函数后等待操作完成 | 注册操作和回调后立即返回 |
| 线程阻塞 | 当前线程可能阻塞在读写操作上 | 当前线程通常不阻塞等待网络 I/O |
| 并发方式 | 当前项目通过一个客户端一个线程实现 | 通过回调和 `io_context` 事件循环实现 |
| 数据处理 | 直接在调用点处理返回值 | 在 handler 中处理错误和传输字节数 |
| 复杂度 | 代码直观，容易理解 | 需要管理回调、生命周期、队列和状态 |
| 适合场景 | 简单程序、学习 socket 流程 | 高并发连接和事件驱动程序 |

## 注意事项

### TCP 没有消息边界

TCP 是字节流协议。一次 `read_some` 可能只读取到部分消息，也可能读取到多条消息的一部分。真实项目需要设计应用层协议，例如：

- 固定长度消息
- 长度字段加消息体
- 特殊分隔符

当前异步示例中的 `RECVSIZE` 是固定缓冲区示例，不等同于完整的通用消息协议。

### buffer 的生命周期

`asio::buffer` 只是对已有内存的视图，不会自动复制数据。异步操作完成前，被引用的字符串、数组或 `MsgNode` 必须保持有效。

### 错误处理

Boost.Asio 的同步和异步 API 都可能返回 `boost::system::error_code`。实际项目中应区分：

- 对端正常关闭连接
- 连接失败
- 读写超时
- 本地 socket 错误
- 异步操作被取消

## 推荐学习顺序

1. 阅读 `同步读写/src/server.cc` 和 `同步读写/src/client.cc`，理解最基本的连接、发送和接收流程。
2. 阅读 `同步读写/src/socket_api.cc`，对比不同同步读写 API。
3. 阅读 `同步读写/src/server_sync.cc`，理解线程和 socket 生命周期。
4. 阅读 `异步读写/include/MsgNode.hpp` 和 `异步读写/src/MsgNode.cc`，理解消息内存管理。
5. 阅读 `异步读写/include/Session.hpp` 和 `异步读写/src/Session.cc`，理解异步发送队列和回调。
6. 为异步模块补充入口程序，再通过 `io_context.run()` 驱动完整通信。

