cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(asio_learn VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system)  #use boost
find_package(Threads REQUIRED) #use thread
find_package(Protobuf CONFIG REQUIRED) #use protocol
find_package(jsoncpp CONFIG REQUIRED) #use jsoncpp

if(TARGET Boost::headers)
    set(ASIO_BOOST_TARGET Boost::headers)
else()
    set(ASIO_BOOST_TARGET Boost::boost)
endif()

set(ASIO_COMMON_LIBS
    ${ASIO_BOOST_TARGET}
    Boost::system
    Threads::Threads
)

# # 01: 同步读写
# # ------------------------------------------------------------------------------
# add_library(sync_socket_api
#     "01_同步读写/src/end_point.cc"
#     "01_同步读写/src/socket_api.cc"
# )
# target_include_directories(sync_socket_api
#     PUBLIC
#         "01_同步读写/include"
# )
# target_link_libraries(sync_socket_api
#     PUBLIC
#         ${ASIO_COMMON_LIBS}
# )
# add_executable(sync_server "01_同步读写/src/server.cc")
# target_include_directories(sync_server PRIVATE "01_同步读写/include")
# target_link_libraries(sync_server PRIVATE ${ASIO_COMMON_LIBS})

# add_executable(sync_client "01_同步读写/src/client.cc")
# target_include_directories(sync_client PRIVATE "01_同步读写/include")
# target_link_libraries(sync_client PRIVATE ${ASIO_COMMON_LIBS})

# add_executable(sync_echo_server "01_同步读写/src/server_sync.cc")
# target_include_directories(sync_echo_server PRIVATE "01_同步读写/include")
# target_link_libraries(sync_echo_server PRIVATE ${ASIO_COMMON_LIBS})

# add_executable(sync_echo_client "01_同步读写/src/client_sync.cc")
# target_include_directories(sync_echo_client PRIVATE "01_同步读写/include")
# target_link_libraries(sync_echo_client PRIVATE ${ASIO_COMMON_LIBS})

# # 02: 异步读写基础组件
# # ------------------------------------------------------------------------------
# add_library(async_session
#     "02_异步读写_api/src/MsgNode.cc"
#     "02_异步读写_api/src/Session.cc"
# )
# target_include_directories(async_session
#     PUBLIC
#         "02_异步读写_api/include"
# )
# target_link_libraries(async_session
#     PUBLIC
#         ${ASIO_COMMON_LIBS}
# )

# # 03: 完整异步读写 Demo
# # ------------------------------------------------------------------------------
# add_library(async_demo_session
#     "03_异步读写_官方demo/src/Session.cc"
# )
# target_include_directories(async_demo_session
#     PUBLIC
#         "03_异步读写_官方demo/include"
# )
# target_link_libraries(async_demo_session
#     PUBLIC
#         ${ASIO_COMMON_LIBS}
# )

# add_library(async_demo_server
#     "03_异步读写_官方demo/src/Server.cc"
# )
# target_include_directories(async_demo_server
#     PUBLIC
#         "03_异步读写_官方demo/include"
# )
# target_link_libraries(async_demo_server
#     PUBLIC
#         async_demo_session
#         ${ASIO_COMMON_LIBS}
# )

# add_executable(async_server
#     "03_异步读写_官方demo/src/async_server.cc"
# )
# target_include_directories(async_server
#     PRIVATE
#         "03_异步读写_官方demo/include"
# )
# target_link_libraries(async_server
#     PRIVATE
#         async_demo_server
# )

# add_executable(sync_client_1
#     "03_异步读写_官方demo/src/Client.cc"
# )
# target_include_directories(sync_client_1
#     PRIVATE
#         "03_异步读写_官方demo/include"
# )
# target_link_libraries(sync_client_1
#     PRIVATE
#         ${ASIO_COMMON_LIBS}
# )

# # 04_异步读写_改进
# # ------------------------------------------------------------------------------
# add_library(async_csession
#     "04_异步读写_改进/src/csession.cc"
#     "04_异步读写_改进/src/msgnode.cc"
# )
# target_include_directories(async_csession
# PUBLIC
#     "04_异步读写_改进/include"
# )
# target_link_libraries(async_csession
#     PUBLIC
#         ${ASIO_COMMON_LIBS}
# )

# add_library(async_cserver
#     "04_异步读写_改进/src/cserver.cc"
# )
# target_include_directories(async_cserver
#     PUBLIC
#         "04_异步读写_改进/include"
# )
# target_link_libraries(async_cserver
#     PUBLIC
#         async_csession
#         ${ASIO_COMMON_LIBS}
# )

# add_executable(cserver_main
#     "04_异步读写_改进/src/async_cserver.cc"
# )
# target_include_directories(cserver_main
#     PRIVATE
#         "04_异步读写_改进/include"
# )
# target_link_libraries(cserver_main
#     PRIVATE
#         async_cserver
# )

# add_executable(cclient
#     "04_异步读写_改进/src/cclient.cc"
# )
# target_include_directories(cclient
#     PRIVATE
#         "04_异步读写_改进/include"
# )
# target_link_libraries(cclient
#     ${ASIO_COMMON_LIBS}
# )

# # 05_protocol
# # ------------------------------------------------------------------------------
# # test
# add_library(protobuf_protocol
#     "05_protocol/test/student.pb.cc"
# )
# target_include_directories(protobuf_protocol
#     PUBLIC
#         "05_protocol/test/"
# )
# target_link_libraries(protobuf_protocol
#     PUBLIC
#         protobuf::libprotobuf
# )
# add_executable(protoc_test
#     "05_protocol/test/test.cc"
# )
# target_include_directories(protoc_test
#     PUBLIC
#     "05_protocol/test/"
# )
# target_link_libraries(protoc_test
#     protobuf_protocol
# )
# # use protocbuf
# add_library(protocbuf_msgnode
#     "05_protocol/src/msgdata.pb.cc"
# )
# target_include_directories(protocbuf_msgnode
# PUBLIC
#     "05_protocol/include/"
# )
# target_link_libraries(protocbuf_msgnode
# PUBLIC
#     protobuf::libprotobuf
# )
# add_library(protocbuf_async_csession
#     "05_protocol/src/csession.cc"
#     "05_protocol/src/msgnode.cc"
# )
# target_include_directories(protocbuf_async_csession
# PUBLIC
#     "05_protocol/include"
# )
# target_link_libraries(protocbuf_async_csession
#     PUBLIC
#         ${ASIO_COMMON_LIBS}
#         protocbuf_msgnode
# )
# add_library(protocbuf_async_cserver
#     "05_protocol/src/cserver.cc"
# )
# target_include_directories(protocbuf_async_cserver
#     PUBLIC
#         "05_protocol/include"
# )
# target_link_libraries(protocbuf_async_cserver
#     PUBLIC
#         protocbuf_async_csession
#         ${ASIO_COMMON_LIBS}
# )
# add_executable(protocbuf_cserver_main
#     "05_protocol/src/async_cserver.cc"
# )
# target_include_directories(protocbuf_cserver_main
#     PRIVATE
#         "05_protocol/include"
# )
# target_link_libraries(protocbuf_cserver_main
#     PRIVATE
#         protocbuf_async_cserver
# )

# add_executable(protocbuf_cclient
#     "05_protocol/src/cclient.cc"
# )
# target_include_directories(protocbuf_cclient
#     PRIVATE
#         "05_protocol/include"
# )
# target_link_libraries(protocbuf_cclient
#     ${ASIO_COMMON_LIBS}
#     protocbuf_msgnode
# )

# # 06_jsoncpp
# # ------------------------------------------------------------------------------
# # test
# add_executable(jsoncpp_test
#     "06_jsoncpp/jsoncpp_test.cc"
# )
# target_link_libraries(jsoncpp_test
#     PRIVATE
#         JsonCpp::JsonCpp
# )
# # use jsoncpp
# add_library(jsoncpp_csession
#     "06_jsoncpp/src/csession.cc"
# )
# target_include_directories(jsoncpp_csession
# PUBLIC
#     "06_jsoncpp/include/"
# )
# target_link_libraries(jsoncpp_csession
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     jsoncpp_msgnode
# )
# add_library(jsoncpp_msgnode
#     "06_jsoncpp/src/msgnode.cc"
# )
# target_include_directories(jsoncpp_msgnode
# PUBLIC
#     "06_jsoncpp/include/"
# )
# target_link_libraries(jsoncpp_msgnode
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )
# add_library(jsoncpp_cserver
#     "06_jsoncpp/src/cserver.cc"
# )
# # target_include_directories(jsoncpp_cserver
# # )
# target_link_libraries(jsoncpp_cserver
# PUBLIC
#     jsoncpp_msgnode
#     jsoncpp_csession
# )
# add_executable(jsoncpp_async_cserver
#     "06_jsoncpp/src/async_cserver.cc"
# )
# target_link_libraries(jsoncpp_async_cserver
#     jsoncpp_cserver
# )
# add_executable(jsoncpp_cclient
#     "06_jsoncpp/src/cclient.cc"
# )
# target_include_directories(jsoncpp_cclient
# PUBLIC
#     "06_jsoncpp/include/"
# )
# target_link_libraries(jsoncpp_cclient
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
# )

# # 07 粘包优化
# add_library(07_csession
#     "07_粘包处理优化/src/csession.cc"
# )
# target_include_directories(07_csession
# PUBLIC
#     "07_粘包处理优化/include/"
# )
# target_link_libraries(07_csession
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     07_msgnode
# )
# add_library(07_msgnode
#     "07_粘包处理优化/src/msgnode.cc"
# )
# target_include_directories(07_msgnode
# PUBLIC
#     "07_粘包处理优化/include/"
# )
# target_link_libraries(07_msgnode
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )
# add_library(07_cserver
#     "07_粘包处理优化/src/cserver.cc"
# )
# # target_include_directories(jsoncpp_cserver
# # )
# target_link_libraries(07_cserver
# PUBLIC
#     07_msgnode
#     07_csession
# )
# add_executable(07_async_cserver
#     "07_粘包处理优化/src/async_cserver.cc"
# )
# target_link_libraries(07_async_cserver
#     07_cserver
# )
# add_executable(07_cclient
#     "07_粘包处理优化/src/cclient.cc"
# )
# target_include_directories(07_cclient
# PUBLIC
#     "07_粘包处理优化/include/"
# )
# target_link_libraries(07_cclient
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
# )

# 08_添加逻辑层 
# 改自 07_粘包处理优化
# add_library(08_csession             
#     "08_添加逻辑层/src/csession.cc"
# )
# target_include_directories(08_csession
# PUBLIC
#     "08_添加逻辑层/include/"
# )
# target_link_libraries(08_csession
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     08_msgnode
#     logic_system
# )

# add_library(08_msgnode
#     "08_添加逻辑层/src/msgnode.cc"
# )
# target_include_directories(08_msgnode
# PUBLIC
#     "08_添加逻辑层/include/"
# )
# target_link_libraries(08_msgnode
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )

# add_library(08_cserver
#     "08_添加逻辑层/src/cserver.cc"
# )
# target_link_libraries(08_cserver
# PUBLIC
#     08_csession
# )

# add_executable(08_async_cserver
#     "08_添加逻辑层/src/async_cserver.cc"
# )
# target_link_libraries(08_async_cserver
#     08_cserver
# )

# add_executable(08_cclient
#     "08_添加逻辑层/src/cclient.cc"
# )
# target_include_directories(08_cclient
# PUBLIC
#     "08_添加逻辑层/include/"
# )
# target_link_libraries(08_cclient
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
# )

# # add_library(singleton
# #     "08_添加逻辑层/src/singleton.cc"
# # )
# # target_include_directories(singleton
# # PUBLIC
# #     "08_添加逻辑层/include/"
# # )

# add_library(logic_system
#     "08_添加逻辑层/src/logic_system.cc"
# )
# target_include_directories(logic_system
# PUBLIC
#     "08_添加逻辑层/include/"
# )
# target_link_libraries(logic_system
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     logic_node
# )

# add_library(logic_node
#     "08_添加逻辑层/src/logic_node.cc"
# )
# target_include_directories(logic_node
# PUBLIC
#     "08_添加逻辑层/include/"
# )
# target_link_libraries(logic_node
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )

# 09_多线程模型_ioServerPoll
# 改自 08_添加逻辑层，增加 io_context 线程池
# add_library(09_msgnode
#     "09_多线程模型_ioServerPoll/src/msgnode.cc"
# )
# target_include_directories(09_msgnode
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_msgnode
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )

# add_library(09_logic_node
#     "09_多线程模型_ioServerPoll/src/logic_node.cc"
# )
# target_include_directories(09_logic_node
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_logic_node
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     09_msgnode
# )

# add_library(09_logic_system
#     "09_多线程模型_ioServerPoll/src/logic_system.cc"
# )
# target_include_directories(09_logic_system
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_logic_system
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     09_logic_node
# )

# add_library(09_csession
#     "09_多线程模型_ioServerPoll/src/csession.cc"
# )
# target_include_directories(09_csession
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_csession
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
#     09_msgnode
#     09_logic_system
# )

# add_library(09_io_context_pool
#     "09_多线程模型_ioServerPoll/src/asio_io_context_poll.cc"
# )
# target_include_directories(09_io_context_pool
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_io_context_pool
# PUBLIC
#     ${ASIO_COMMON_LIBS}
# )

# add_library(09_cserver
#     "09_多线程模型_ioServerPoll/src/cserver.cc"
# )
# target_include_directories(09_cserver
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_cserver
# PUBLIC
#     09_csession
#     09_io_context_pool
# )

# add_executable(09_async_cserver
#     "09_多线程模型_ioServerPoll/src/async_cserver.cc"
# )
# target_link_libraries(09_async_cserver
# PRIVATE
#     09_cserver
# )

# add_executable(09_cclient
#     "09_多线程模型_ioServerPoll/src/cclient.cc"
# )
# target_include_directories(09_cclient
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_cclient
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
# )

# add_executable(09_cclient_test
#     "09_多线程模型_ioServerPoll/src/cclient_test.cc"
# )
# target_include_directories(09_cclient_test
# PUBLIC
#     "09_多线程模型_ioServerPoll/include/"
# )
# target_link_libraries(09_cclient_test
# PUBLIC
#     ${ASIO_COMMON_LIBS}
#     JsonCpp::JsonCpp
# )

# 10_多线程模型_ioThreadPoll
# 使用线程池运行 io_context
add_library(10_msgnode
    "10_多线程模型_ioThreadPoll/src/msgnode.cc"
)
target_include_directories(10_msgnode
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_msgnode
PUBLIC
    ${ASIO_COMMON_LIBS}
)

add_library(10_logic_node
    "10_多线程模型_ioThreadPoll/src/logic_node.cc"
)
target_include_directories(10_logic_node
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_logic_node
PUBLIC
    ${ASIO_COMMON_LIBS}
    10_msgnode
)

add_library(10_logic_system
    "10_多线程模型_ioThreadPoll/src/logic_system.cc"
)
target_include_directories(10_logic_system
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_logic_system
PUBLIC
    ${ASIO_COMMON_LIBS}
    JsonCpp::JsonCpp
    10_logic_node
)

add_library(10_csession
    "10_多线程模型_ioThreadPoll/src/csession.cc"
)
target_include_directories(10_csession
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_csession
PUBLIC
    ${ASIO_COMMON_LIBS}
    JsonCpp::JsonCpp
    10_msgnode
    10_logic_system
)

add_library(10_io_thread_pool
    "10_多线程模型_ioThreadPoll/src/asio_thread_poll.cc"
)
target_include_directories(10_io_thread_pool
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_io_thread_pool
PUBLIC
    ${ASIO_COMMON_LIBS}
)

add_library(10_cserver
    "10_多线程模型_ioThreadPoll/src/cserver.cc"
)
target_include_directories(10_cserver
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_cserver
PUBLIC
    10_csession
    10_io_thread_pool
)

add_executable(10_async_cserver
    "10_多线程模型_ioThreadPoll/src/async_cserver.cc"
)
target_link_libraries(10_async_cserver
PRIVATE
    10_cserver
)

add_executable(10_cclient
    "10_多线程模型_ioThreadPoll/src/cclient.cc"
)
target_include_directories(10_cclient
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_cclient
PUBLIC
    ${ASIO_COMMON_LIBS}
    JsonCpp::JsonCpp
)

add_executable(10_cclient_test
    "10_多线程模型_ioThreadPoll/src/cclient_test.cc"
)
target_include_directories(10_cclient_test
PUBLIC
    "10_多线程模型_ioThreadPoll/include/"
)
target_link_libraries(10_cclient_test
PUBLIC
    ${ASIO_COMMON_LIBS}
    JsonCpp::JsonCpp
)
