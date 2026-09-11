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

add_library(async_session
    "02_异步读写/src/MsgNode.cc"
    "02_异步读写/src/Session.cc"
)
target_include_directories(async_session
    PUBLIC
        "02_异步读写/include"
)
target_link_libraries(async_session
    PUBLIC
        ${ASIO_COMMON_LIBS}
)
