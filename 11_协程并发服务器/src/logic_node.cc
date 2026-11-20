#include "logic_node.hpp"
#include "session.hpp"
#include "msg_node.hpp"
#include <memory>

logic_node::logic_node(std::shared_ptr<session> session,std::shared_ptr<recv_msgnode> recv_node):
    _session(session),
    _recv_node(recv_node){
    }

