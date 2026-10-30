#pragma once
#include "csession.hpp"
#include "msgnode.hpp"
#include <memory>

class logic_system;

class logic_node{
    friend logic_system;
public:
    logic_node(std::shared_ptr<csession>,std::shared_ptr<recv_msgnode>);
private:
    std::shared_ptr<csession> _session;
    std::shared_ptr<recv_msgnode> _recv_node;
};