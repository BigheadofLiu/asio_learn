# Boost.Asio 网络编程学习项目

使用 C++20、Boost.Asio 和 CMake 学习 TCP 网络编程。项目按章节保存演进过程：同步读写、异步回调、消息封包、序列化、逻辑层、多线程模型，以及协程读取。

当前根目录的 `CMakeLists.txt` 默认启用第 11 章，01～10 章的目标定义保留为注释。

## 环境与依赖

| 依赖 | 用途 |
|---|---|
| 支持 C++20 协程的 C++ 编译器 | 编译 `co_await` 等协程代码 |
| CMake 3.16+ | 配置与构建 |
| Boost / Boost.System | Asio 网络、错误码、字节序转换等 |
| 系统线程库 | I/O 工作线程和逻辑线程 |
| Protobuf 与 `protoc` | 第 05 章的消息序列化及代码生成 |
| JsonCpp | JSON 消息解析与序列化 |

当前代码包含 POSIX 头文件，主要面向 macOS / Linux。

macOS 可通过 Homebrew 安装依赖：

```bash
brew install cmake boost protobuf jsoncpp
```

根 CMake 无条件查找 Protobuf 和 JsonCpp，因此即使只构建第 11 章，也需要安装这两个依赖。

## 项目目录与学习顺序

```text
asio_learn/
├── CMakeLists.txt
├── ReadMe.md
├── 01_同步读写/
├── 02_异步读写_api/
├── 03_异步读写_官方demo/
├── 04_异步读写_改进/
├── 05_protocol/
├── 06_jsoncpp/
├── 07_粘包处理优化/
├── 08_添加逻辑层/
├── 09_多线程模型_ioServerPoll/
├── 10_多线程模型_ioThreadPoll/
└── 11_协程并发服务器/
```

多数章节采用 `include/` 放头文件、`src/` 放实现文件的结构。`05_protocol/` 还包含协议定义及测试，`11_协程并发服务器/` 额外提供独立的 `demo/`。

| 章节 | 主要内容 | 推荐阅读入口 |
|---|---|---|
| [01：同步读写](01_同步读写/) | endpoint、socket、阻塞读写、每连接一个线程的 Echo 示例 | `src/server.cc`、`src/server_sync.cc` |
| [02：异步读写 API](02_异步读写_api/) | 消息节点、部分读写、buffer 生命周期与发送队列；本章是组件库 | `src/Session.cc`、`src/MsgNode.cc` |
| [03：官方异步 Demo](03_异步读写_官方demo/) | 组合 Server、Session 和异步回调，实现 Echo 服务 | `src/async_server.cc`、`src/Session.cc` |
| [04：异步读写改进](04_异步读写_改进/) | 长度头协议、半包与粘包处理、连接生命周期 | `src/csession.cc`、`src/msgnode.cc` |
| [05：Protobuf](05_protocol/) | 将 Protobuf 序列化数据放入 TCP 消息正文 | `msgdata.proto`、`src/cclient.cc`、`test/test.cc` |
| [06：JsonCpp](06_jsoncpp/) | JSON 序列化、解析及网络收发 | `jsoncpp_test.cc`、`src/csession.cc` |
| [07：粘包处理优化](07_粘包处理优化/) | 通过接收进度与状态处理跨回调消息重组 | `src/csession.cc` |
| [08：添加逻辑层](08_添加逻辑层/) | 消息 ID、业务回调分发、逻辑队列与逻辑线程 | `src/logic_system.cc`、`src/logic_node.cc` |
| [09：多个 io_context](09_多线程模型_ioServerPoll/) | 每个工作线程运行自己的 io_context，轮询分配连接 | `src/asio_io_context_poll.cc` |
| [10：共享 io_context](10_多线程模型_ioThreadPoll/) | 多个线程运行同一个 io_context，使用 strand 串行化会话处理 | `src/asio_thread_poll.cc`、`src/csession.cc` |
| [11：协程并发服务器](11_协程并发服务器/) | 保留线程池和逻辑层，将会话读取回调链改为协程循环 | `src/async_server.cc`、`src/session.cc` |

## 构建

在项目根目录执行：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

如果 CMake 找不到 Homebrew 安装的依赖，可显式提供搜索路径：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$(brew --prefix)"
```

仅构建指定程序：

```bash
cmake --build build --target 11_async_server 11_client_test --parallel
```

仅构建协程 Echo Demo：

```bash
cmake --build build --target 11_coroutine_server_demo 11_client_demo --parallel
```

01～10 章当前没有启用，直接构建其目标会提示目标不存在。要学习旧章节，先取消根 `CMakeLists.txt` 中对应章节及其依赖目标的注释，再重新运行 CMake 配置。

## 先建立 Boost.Asio 的整体认识

学习这个项目时，可以先把 Asio 看成三个部分：

```text
I/O 对象             异步操作               事件循环
socket / acceptor  → async_read/...      → io_context::run()
```

- `io_context`：异步任务的调度中心。异步操作注册后，需要线程调用 `run()` 才会执行完成处理。
- `tcp::acceptor`：服务端监听 socket，负责接受新连接。
- `tcp::socket`：代表一个 TCP 连接，负责读写字节。
- `endpoint`：IP 地址与端口的组合。
- `buffer`：对现有内存的视图，不拥有内存。异步操作完成前，底层内存必须保持有效。
- completion handler：异步操作完成后执行的回调。
- executor：描述任务交给谁执行；`io_context`、socket 和 strand 都能提供 executor。
- strand：让绑定到同一 strand 的处理串行执行，避免它们在多个 `run()` 线程上同时运行。

同步操作会阻塞当前线程：

```cpp
acceptor.accept(socket);
auto size = socket.read_some(buffer);
boost::asio::write(socket, buffer);
```

异步操作立即返回，完成后由 `io_context` 调用回调：

```cpp
socket.async_read_some(buffer, handler);
ioc.run();
```

协程仍然使用异步操作，只是把回调流程写成顺序形式：

```cpp
auto size = co_await socket.async_read_some(
    buffer,
    boost::asio::use_awaitable
);
```

`co_await` 暂停的是当前协程，不是执行 `io_context::run()` 的线程。

## 第 01 章：同步读写

目录：[01_同步读写](01_同步读写/)

### 学习目标

先不用回调和协程，理解 TCP 连接的基本组成：

```text
服务端                               客户端
io_context                           io_context
    ↓                                    ↓
acceptor(open/bind/listen)            socket
    ↓                                    ↓
accept()  ←──────── connect() ──────────┘
    ↓
得到通信 socket
    ↓
read / write
```

`socket_api.cc` 将 Asio API 和原生 Socket API 对照起来：

| Boost.Asio | 对应概念 |
|---|---|
| `tcp::endpoint` | IP 地址和端口 |
| `tcp::socket` | 通信 socket |
| `tcp::acceptor` | `socket + bind + listen` |
| `socket.connect()` | 客户端连接 |
| `acceptor.accept()` | 服务端接受连接 |
| `tcp::resolver` | 域名解析 |
| `asio::buffer()` | 将已有内存包装为缓冲区视图 |

### `read_some` 与 `read`

```cpp
auto n = socket.read_some(boost::asio::buffer(data));
```

`read_some` 只保证读取“当前能取得的一部分”，返回值 `n` 才是有效字节数。一次调用不保证读满 buffer。

```cpp
auto n = boost::asio::read(socket, boost::asio::buffer(data));
```

`asio::read` 是组合操作，内部可能执行多次 `read_some`，直到满足指定长度或出现错误。

写操作同理：

- `socket.write_some()`：可能只写出一部分。
- `boost::asio::write()`：组合多次写操作，尽量完成整个 buffer。

### 为什么每连接一个线程

同步 `accept()` 和 `read_some()` 都会阻塞。如果主线程直接处理一个客户端，就无法继续接受其他客户端。因此 `server_sync.cc` 在接受连接后创建线程：

```text
主线程：accept A → 创建线程 A → accept B → 创建线程 B
线程 A：阻塞处理连接 A
线程 B：阻塞处理连接 B
```

这种模型容易理解，但连接很多时，线程数量、栈内存和线程切换成本都会增加。这正是后续异步模型要解决的问题。

### 本章建议重点

1. 先读 `socket_api.cc`，认识 endpoint、socket、acceptor、resolver 和 buffer。
2. 再读 `server_sync.cc`，跟踪一次连接从 accept 到 EOF 的完整过程。
3. 牢记 TCP 是字节流：一次发送和一次接收没有天然的一一对应关系。

## 第 02 章：异步读写 API

目录：[02_异步读写_api](02_异步读写_api/)

### 为什么需要异步

异步操作发起后立即返回，线程可以继续处理其他连接：

```text
发起 async_read
    ↓
函数立即返回
    ↓
数据到达
    ↓
io_context 调用 ReadCallBack
```

本章是组件练习，没有完整服务器入口。核心类是 `Session` 和 `MsgNode`。

### buffer 生命周期

`asio::buffer()` 不复制数据，只保存地址和长度：

```cpp
socket.async_write_some(
    boost::asio::buffer(data),
    handler
);
```

在 `handler` 执行前，`data` 必须仍然存在。不能把异步操作绑定到即将离开作用域的局部字符串或数组。本章用 `MsgNode` 保存：

```text
数据地址 + 总长度 + 已处理长度
```

并由 `shared_ptr` 延长消息节点的生命周期。

### 部分读写

`async_write_some` 可能只发送一部分，因此回调要推进偏移量：

```text
第一次发送：0 ........ n
第二次发送：n ........ total
```

关键状态是：

```cpp
_cur_length += bytes_transferred;
```

如果 `_cur_length < _total_length`，继续发送剩余部分。

`boost::asio::async_write` 是组合异步操作，会在内部处理部分写，完成回调通常只需处理“整段完成”。

### 为什么需要发送队列

同一个 socket 不应同时启动多个相互独立的写操作，否则消息顺序和 buffer 生命周期难以保证：

```text
send(A) → A 入队，启动异步写
send(B) → B 入队，不启动第二个写
A 完成  → A 出队，开始写 B
```

`_send_pending` 表示是否已有写操作正在进行。

### 本章建议重点

- 对比 `WriteCallBack` 与 `WriteALLCallBack`。
- 对比 `ReadFromSocket` 与 `ReadALlFromSocket`。
- 理解“异步”不等于“自动创建线程”，回调仍由调用 `io_context::run()` 的线程执行。

## 第 03 章：完整异步 Echo Server

目录：[03_异步读写_官方demo](03_异步读写_官方demo/)

这一章把第 02 章的概念组合成可运行结构：

```text
main
 └── io_context::run()
      └── Server::async_accept()
           └── 创建 Session
                ├── async_read_some()
                ├── handle_read()
                ├── async_write()
                └── handle_write()
```

### `enable_shared_from_this`

异步操作完成前，`Session` 必须存在。类继承：

```cpp
std::enable_shared_from_this<Session>
```

回调绑定时取得：

```cpp
auto self = shared_from_this();
```

只要回调持有 `self`，Session 就不会提前析构。前提是对象本身最初由 `std::shared_ptr` 创建。

### Echo 循环

```text
start
  ↓
async_read_some
  ↓
handle_read：按 bytes_transferred 回写
  ↓
async_write
  ↓
handle_write：再次注册 async_read_some
```

异步服务器必须在完成回调中注册下一次读取，才能维持长连接。

### 本章限制

这一章主要讲异步流程，还没有应用层消息协议。它只把当前读取到的字节原样回写，不能把“一次回调”当作“一条完整业务消息”。

## 第 04 章：长度头、半包与粘包

目录：[04_异步读写_改进](04_异步读写_改进/)

### 为什么需要消息协议

TCP 只传输连续字节：

```text
发送： [消息 A] [消息 B]
接收： [A 的一部分] [A 剩余 + B] ...
```

因此项目定义：

```text
[4 字节正文长度][正文]
```

长度字段使用 `std::uint32_t`，发送前转换成大端网络字节序：

```cpp
auto network_length =
    boost::endian::native_to_big(local_length);
```

接收后再转换回本机字节序。

### 状态机如何重组消息

`async_read_some` 给出本批次有效字节数。接收逻辑用以下状态跨回调保存进度：

- `_recv_head_node`：已收到的消息头字节。
- `_recv_msg_node`：已收到的消息正文字节。
- `_head_status`：头部是否完整，下一步该解析头还是继续正文。
- `copy_length`：当前临时 buffer 已消费到的位置。
- `byte_transfered`：当前临时 buffer 还剩多少有效字节。

一批数据可能包含多个消息，因此处理完一条后要继续消费剩余字节：

```text
while (当前批次还有字节)
    补齐消息头
    解析并校验长度
    补齐正文
    完整后处理消息
    重置状态，继续解析下一条
```

这里最容易出错的是偏移量未推进，导致重复复制或死循环。

### 本章建议重点

- 不要用 `strlen()` 处理网络正文，正文可以包含 `\0`。
- `memcpy` 前先验证协议长度和目标 buffer 容量。
- `size_t` 只用于本机长度；线协议使用固定宽度整数。
- 断开连接后不要再调用 `remote_endpoint()` 获取对端地址。

## 第 05 章：Protobuf 消息体

目录：[05_protocol](05_protocol/)

本章保留第 04 章的长度头，将正文换成 Protobuf 二进制：

```text
[uint32 正文长度][Protobuf 序列化结果]
```

### Protobuf 流程

```text
.proto
  ↓ protoc
.pb.h / .pb.cc
  ↓
构造 C++ 消息对象
  ↓ SerializeToString
二进制正文
  ↓ TCP
ParseFromArray / ParseFromString
  ↓
恢复消息对象
```

示例：

```cpp
msgdata message;
message.set_id(1001);
message.set_data("hello world");

std::string body;
if (!message.SerializeToString(&body)) {
    // 序列化失败
}
```

接收端按协议长度读出正文后再反序列化：

```cpp
msgdata message;
if (!message.ParseFromArray(data, length)) {
    // 协议内容无效
}
```

只对长度头进行网络字节序转换。Protobuf 正文已经由 Protobuf 定义编码格式，不应再对正文逐字段转换字节序。

## 第 06 章：JsonCpp

目录：[06_jsoncpp](06_jsoncpp/)

本章将业务对象表示为 JSON 文本。`jsoncpp_test.cc` 演示最小流程：

```cpp
Json::Value root;
root["name"] = "demo";
root["id"] = 1001;

std::string text = root.toStyledString();
```

解析：

```cpp
Json::Reader reader;
Json::Value root;
if (!reader.parse(text, root)) {
    // JSON 无效
}
```

JSON 易于观察和调试，但文本通常比 Protobuf 大。两者都只是“正文的表示方式”，仍需 TCP 消息头解决边界问题：

```text
TCP 消息协议：负责从字节流切出一条正文
JSON / Protobuf：负责解释这段正文的字段
```

使用 JsonCpp 的可执行目标不仅需要包含 `<json/json.h>`，还必须在 CMake 中链接 `JsonCpp::JsonCpp`。

## 第 07 章：粘包处理优化

目录：[07_粘包处理优化](07_粘包处理优化/)

第 04 章手动用 `async_read_some + 状态机` 消费临时 buffer。第 07 章改为两个固定步骤：

```text
async_read(固定 4 字节头)
    ↓
解析正文长度并校验
    ↓
async_read(指定长度正文)
    ↓
处理消息
    ↓
再次 async_read(消息头)
```

`boost::asio::async_read` 是组合操作，内部会反复调用底层 `async_read_some`，直到读满指定 buffer。因此代码不再需要手动处理头部和正文的半包进度，结构更清晰。

需要注意：

- `async_read` 解决的是“读满指定长度”，消息协议仍然由程序定义。
- 读取正文前必须先检查长度上限。
- 同一 socket 同一方向不要同时启动两条读取链。
- 每次正文处理完成后，必须重新注册下一次头部读取。

## 第 08 章：消息 ID 与逻辑层

目录：[08_添加逻辑层](08_添加逻辑层/)

协议增加消息 ID：

```text
[uint16 消息 ID][uint32 正文长度][正文]
```

ID 和长度都使用大端网络字节序。ID 用于决定由哪个业务处理函数消费正文。

### 为什么把网络层和业务层分开

```text
网络线程
    ├── 接受连接
    ├── 读写 socket
    └── 拆分完整消息
             ↓ 投递 logic_node
逻辑线程
    ├── 从队列取消息
    ├── 按 msg_id 查找回调
    └── 执行业务并调用 session::send()
```

网络层不需要知道“登录、聊天、心跳”等业务含义，只负责得到：

```text
session + msg_id + 完整正文
```

`logic_system` 使用回调表分发：

```cpp
callbacks[msg_id] = handler;
```

队列为空时，逻辑线程通过 `condition_variable` 等待；生产者加入第一条消息后通知它。停止时设置标志、通知线程并在析构中 `join()`。

### 本章建议重点

- `logic_node` 为什么同时保存 Session 和消息节点。
- 队列为什么需要互斥锁。
- 条件变量为什么必须配合条件循环使用。
- 未注册的消息 ID 应丢弃或返回明确错误。
- 友元关系不继承、不传递；更推荐为消息节点提供只读访问器。

## 第 09 章：多个 io_context 的 I/O 池

目录：[09_多线程模型_ioServerPoll](09_多线程模型_ioServerPoll/)

该模型创建多个 `io_context`，每个 `io_context` 通常由一个工作线程运行：

```text
io_context 0 ← thread 0
io_context 1 ← thread 1
io_context 2 ← thread 2
...
```

新连接按轮询方式分配：

```text
连接 A → io_context 0
连接 B → io_context 1
连接 C → io_context 2
连接 D → io_context 0
```

一个连接的 socket 绑定到选中的 `io_context`，该连接的完成处理通常就在对应工作线程执行。这样降低同一 Session 被多个 I/O 线程同时执行的可能性。

### `executor_work_guard`

如果 `io_context` 暂时没有任务，`run()` 会返回，工作线程随之退出。work guard 表示“未来还会有任务”，让 `run()` 保持等待：

```cpp
boost::asio::make_work_guard(ioc);
```

停止池时通常按顺序执行：

```text
reset work guard
    ↓
stop io_context
    ↓
join 工作线程
```

轮询下标如果可能被多个线程调用，需要额外同步；当前项目主要从接受连接的路径中使用它。

## 第 10 章：共享 io_context 的线程池

目录：[10_多线程模型_ioThreadPoll](10_多线程模型_ioThreadPoll/)

这一章只创建一个 `io_context`，多个线程同时调用同一个 `run()`：

```text
                 ┌── thread 0: ioc.run()
同一个 io_context ├── thread 1: ioc.run()
                 └── thread 2: ioc.run()
```

优点是任务可以自然分配到空闲线程；代价是两个完成处理可能同时访问同一个 Session。

### 为什么需要 strand

Session 创建自己的 strand：

```cpp
boost::asio::strand<
    boost::asio::io_context::executor_type
> strand;
```

将读写完成处理绑定到该 strand：

```cpp
boost::asio::bind_executor(strand, handler);
```

同一个 strand 上的 handler 不会并发执行：

```text
handle_read ──执行完──> handle_write ──执行完──> ...
```

strand 只保证通过它调度的处理串行执行。若其他业务线程直接调用 `send()` 并操作 `_send_queue`，仍需要互斥锁，或者先把操作 `post` 到 strand。

### 第 09 与第 10 章对比

| 对比项 | 第 09 章 | 第 10 章 |
|---|---|---|
| `io_context` 数量 | 多个 | 一个 |
| 每个 `io_context` 的线程 | 通常一个 | 多个线程共享 |
| 连接分配 | 轮询选择 `io_context` | 全部使用同一个 `io_context` |
| Session 并发风险 | 较低 | 更高 |
| 主要串行手段 | 连接固定到工作 context | strand |

理解这两章后，再看第 11 章的“多 io_context 工作池 + Session strand + 协程”，会更容易理解各部分职责。

## 第 11 章：目录与构建目标

```text
11_协程并发服务器/
├── demo/
│   ├── coroutine_server_demo.cc  # 原始字节 Echo 服务器
│   └── client_demo.cc            # 配套交互客户端
├── include/
│   ├── const.hpp                # 协议长度、正文和队列上限
│   ├── singleton.hpp            # 单例模板
│   ├── io_context_poll.hpp      # 多 io_context 工作池
│   ├── msg_node.hpp             # 发送与接收消息节点
│   ├── logic_node.hpp           # 会话和接收消息的组合
│   ├── logic_system.hpp         # 业务分发及逻辑线程
│   ├── session.hpp              # 单个 TCP 连接的读写
│   └── server.hpp               # 监听与连接管理
└── src/
    ├── async_server.cc          # 主服务器入口
    ├── client.cc                # 多线程 JSON 测试客户端
    ├── io_context_poll.cc
    ├── msg_node.cc
    ├── logic_node.cc
    ├── logic_system.cc
    ├── session.cc
    └── server.cc
```

根 CMake 按模块分别定义目标：

| 目标 | 类型 | 用途 |
|---|---|---|
| `11_singleton` | INTERFACE 库 | 单例模板头文件 |
| `11_constants` | INTERFACE 库 | 协议常量头文件 |
| `11_msg_node` | 库 | 消息缓冲区及封包 |
| `11_io_context_pool` | 库 | I/O 工作线程池 |
| `11_logic_node` | 库 | 逻辑消息节点 |
| `11_logic_system` | 库 | JSON 业务处理及分发 |
| `11_session` | 库 | 连接读取与发送队列 |
| `11_server` | 库 | TCP accept 与会话管理 |
| `11_coroutine_server_demo` | 可执行程序 | 独立协程 Echo 服务器 |
| `11_client_demo` | 可执行程序 | Echo 客户端 |
| `11_async_server` | 可执行程序 | 带消息协议及逻辑层的服务器 |
| `11_client_test` | 可执行程序 | 配套多线程测试客户端 |

## 运行第 11 章

### 原始字节 Echo 示例

终端一：

```bash
./build/11_coroutine_server_demo
```

终端二：

```bash
./build/11_client_demo
```

客户端连接 `127.0.0.1:8899`，输入一条非空消息后，服务端原样回显，客户端收到回复后退出。

### 带协议和逻辑层的服务器

终端一：

```bash
./build/11_async_server
```

终端二：

```bash
./build/11_client_test
```

测试客户端连接 `127.0.0.1:8899`。当前源码创建 100 个线程，每个线程建立一个连接，循环进行 500 次 JSON 请求与响应；这些数量在 `src/client.cc` 中设置。完成后输出耗时，并等待按回车。

当前注册的业务处理器是消息 ID `1001`，解析 JSON 中的 `id` 和 `data`，再将 JSON 回复给客户端。`1002`、`1003` 已定义为消息 ID，但当前没有注册对应处理器。

两个服务器都监听 `8899`，请分别运行。Echo Demo 和 JSON 服务器的消息格式不同，客户端需按上述组合配对。

主服务器使用 `SIGINT` / `SIGTERM` 处理退出信号，可通过 `Ctrl+C` 停止。

## 第 11 章的执行流程

```text
主线程：独立 io_context
    └── server：监听 8899，异步接受连接
            └── 从 io_context_poll 轮询获取工作 io_context
                    └── 创建 session
                            └── 在 session 的 strand 上启动 read_loop
                                    ├── co_await 读取完整消息头
                                    ├── 解析 ID 和正文长度
                                    ├── 校验正文长度
                                    ├── co_await 读取完整正文
                                    └── 投递 logic_node 到逻辑队列

逻辑线程
    └── logic_system 按消息 ID 分发业务
            └── session::send 将回复加入发送队列
                    └── async_write + 完成回调逐条发送
```

当前实现是“协程读取 + 回调发送”。服务器接受连接也仍使用异步回调，尚未全部改成协程。`session.cc` 中保留了旧读取回调作为演进代码，当前 `start()` 启动的是 `read_loop()`。

几个关键概念：

- `io_context::run()` 驱动异步操作与完成处理。
- `co_spawn()` 启动协程，不等同于创建线程。
- `co_await` 在等待 I/O 时可暂停当前协程，让线程执行其他任务。
- `use_awaitable` 将 Asio 异步操作接入协程，错误默认以异常报告。
- 同一个 strand 上的处理不会同时执行；它不会自动保护其他线程直接调用的成员函数。
- 协程按值持有 `shared_ptr<session>`，延长会话生命周期。
- 发送队列保持同一连接一次只有一个活跃的异步写操作。

## 消息协议

第 11 章主服务器使用以下格式：

```text
[2 字节消息 ID][4 字节正文长度][正文]
```

| 字段 | 类型或规则 |
|---|---|
| 消息 ID | `std::uint16_t`，大端网络字节序 |
| 正文长度 | `std::uint32_t`，大端网络字节序；不包含头部 |
| 正文 | 按长度传输的字节数据，当前业务使用 JSON |
| 消息头总长 | 6 字节 |
| 正文最大长度 | 2048 字节，定义在 `include/const.hpp` |
| 发送队列上限常量 | 1000，定义在 `include/const.hpp` |

`std::size_t` 用于本地缓冲区容量、偏移和字节数，不直接作为网络字段发送。网络序长度用于写入协议头，内存复制与缓冲区大小使用本机长度。

TCP 不保留消息边界。第 11 章使用 `async_read` 先读完整头部，再按解析出的长度读完整正文，因此不要求一次底层接收恰好得到一条消息。

旧章节采用不同协议：

- 04～07 章：`[4 字节正文长度][正文]`；05 的正文为 Protobuf，06～07 使用 JSON。
- 08～11 章主服务器：增加消息 ID，使用 `[2 字节 ID][4 字节正文长度][正文]`。
- 11 章 Echo Demo：直接收发原始字节，不使用上述消息头。

不能仅凭端口相同混用不同章节的客户端与服务器。

## Protobuf 示例

第 05 章保留了 `.proto` 和生成的 C++ 文件：

```text
05_protocol/
├── msgdata.proto
├── include/msgdata.pb.h
├── src/msgdata.pb.cc
└── test/
    ├── student.proto
    ├── student.pb.h
    ├── student.pb.cc
    └── test.cc
```

修改协议后，可在项目根目录重新生成：

```bash
protoc -I 05_protocol --cpp_out=05_protocol 05_protocol/msgdata.proto
mv 05_protocol/msgdata.pb.h 05_protocol/include/
mv 05_protocol/msgdata.pb.cc 05_protocol/src/

protoc -I 05_protocol/test --cpp_out=05_protocol/test \
  05_protocol/test/student.proto
```

生成代码应与当前安装的 Protobuf 库版本兼容。

## 常见构建与运行问题

- **`Undefined symbols` 中出现 `Json::...`**：使用 JSON 的目标需要链接 `JsonCpp::JsonCpp`；仅包含头文件不够。
- **缺少 `server::~server()` 等符号**：检查函数是否只有声明，以及实现文件是否加入对应目标。
- **`ignoring duplicate libraries`**：当前 `11_logic_system` 和 `11_session` 存在双向静态库依赖，链接命令可能重复列出它们。
- **依赖库要求更高 macOS 版本**：检查构建部署版本与 Homebrew 库的最低系统版本是否匹配。
- **`Address already in use`**：确认另一个章节的服务器没有占用 `8899`。
- **`End of file` / EOF**：通常表示对端关闭了发送方向；连接结束时应退出读循环并清理会话。

这是按教程演进的学习项目。构建成功不代表并发、退出流程和全部协议边界已验证；阅读时可重点关注共享容器的同步、异步对象生命周期以及异常路径的资源清理。
