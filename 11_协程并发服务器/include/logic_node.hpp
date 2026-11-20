#pragma once
#include "session.hpp"
#include "msg_node.hpp"
#include <memory>

class logic_system;

class logic_node{
    friend logic_system;
public:
    logic_node(std::shared_ptr<session>,std::shared_ptr<recv_msgnode>);
private:
    std::shared_ptr<session> _session;
    std::shared_ptr<recv_msgnode> _recv_node;
};