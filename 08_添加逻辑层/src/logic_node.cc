#include "logic_node.hpp"
#include "csession.hpp"
#include "msgnode.hpp"
#include <memory>

logic_node::logic_node(std::shared_ptr<csession> session,std::shared_ptr<recv_msgnode> recv_node):
    _session(session),
    _recv_node(recv_node){

    }

