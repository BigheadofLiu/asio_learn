cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(asio_learn VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

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

# 01: 同步读写
# ------------------------------------------------------------------------------
add_library(sync_socket_api
    "01_同步读写/src/end_point.cc"
    "01_同步读写/src/socket_api.cc"
)
target_include_directories(sync_socket_api
    PUBLIC
        "01_同步读写/include"
)
target_link_libraries(sync_socket_api
    PUBLIC
        ${ASIO_COMMON_LIBS}
)
add_executable(sync_server "01_同步读写/src/server.cc")
target_include_directories(sync_server PRIVATE "01_同步读写/include")
target_link_libraries(sync_server PRIVATE ${ASIO_COMMON_LIBS})

add_executable(sync_client "01_同步读写/src/client.cc")
target_include_directories(sync_client PRIVATE "01_同步读写/include")
target_link_libraries(sync_client PRIVATE ${ASIO_COMMON_LIBS})

add_executable(sync_echo_server "01_同步读写/src/server_sync.cc")
target_include_directories(sync_echo_server PRIVATE "01_同步读写/include")
target_link_libraries(sync_echo_server PRIVATE ${ASIO_COMMON_LIBS})

add_executable(sync_echo_client "01_同步读写/src/client_sync.cc")
target_include_directories(sync_echo_client PRIVATE "01_同步读写/include")
target_link_libraries(sync_echo_client PRIVATE ${ASIO_COMMON_LIBS})

# 02: 异步读写基础组件
# ------------------------------------------------------------------------------
add_library(async_session
    "02_异步读写_api/src/MsgNode.cc"
    "02_异步读写_api/src/Session.cc"
)
target_include_directories(async_session
    PUBLIC
        "02_异步读写_api/include"
)
target_link_libraries(async_session
    PUBLIC
        ${ASIO_COMMON_LIBS}
)

# 03: 完整异步读写 Demo
# ------------------------------------------------------------------------------
add_library(async_demo_session
    "03_异步读写_官方demo/src/Session.cc"
)
target_include_directories(async_demo_session
    PUBLIC
        "03_异步读写_官方demo/include"
)
target_link_libraries(async_demo_session
    PUBLIC
        ${ASIO_COMMON_LIBS}
)

add_library(async_demo_server
    "03_异步读写_官方demo/src/Server.cc"
)
target_include_directories(async_demo_server
    PUBLIC
        "03_异步读写_官方demo/include"
)
target_link_libraries(async_demo_server
    PUBLIC
        async_demo_session
        ${ASIO_COMMON_LIBS}
)

add_executable(async_server
    "03_异步读写_官方demo/src/async_server.cc"
)
target_include_directories(async_server
    PRIVATE
        "03_异步读写_官方demo/include"
)
target_link_libraries(async_server
    PRIVATE
        async_demo_server
)

add_executable(sync_client_1
    "03_异步读写_官方demo/src/Client.cc"
)
target_include_directories(sync_client_1
    PRIVATE
        "03_异步读写_官方demo/include"
)
target_link_libraries(sync_client_1
    PRIVATE
        ${ASIO_COMMON_LIBS}
)

# 04_异步读写_改进
# ------------------------------------------------------------------------------
add_library(async_csession
    "04_异步读写_改进/src/csession.cc"
    "04_异步读写_改进/src/msgnode.cc"
)
target_include_directories(async_csession
PUBLIC
    "04_异步读写_改进/include"
)
target_link_libraries(async_csession
    PUBLIC
        ${ASIO_COMMON_LIBS}
)

add_library(async_cserver
    "04_异步读写_改进/src/cserver.cc"
)
target_include_directories(async_cserver
    PUBLIC
        "04_异步读写_改进/include"
)
target_link_libraries(async_cserver
    PUBLIC
        async_csession
        ${ASIO_COMMON_LIBS}
)

add_executable(cserver_main
    "04_异步读写_改进/src/async_cserver.cc"
)
target_include_directories(cserver_main
    PRIVATE
        "04_异步读写_改进/include"
)
target_link_libraries(cserver_main
    PRIVATE
        async_cserver
)