#pragma once
#include <cstddef>
#include <cstdint>

namespace constants{
    //[消息id ｜ 消息头 ｜ 消息正文]
    inline constexpr std::size_t msg_body_max_size = 1024 * 2;   //消息正文最大长度
    inline constexpr std::size_t msg_head_length = sizeof(std::uint32_t);
    inline constexpr std::size_t msg_id_length = sizeof(std::uint16_t);
    inline constexpr std::size_t msg_head_total_length = msg_head_length + msg_id_length;
    inline constexpr std::size_t send_queue_max_size=1000;
    inline constexpr std::size_t recv_queue_max_size=10000;
}

