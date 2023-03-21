#include "machinepool.h"

MachinePool::MachinePool(int port)
{
    this->listen(QHostAddress::Any,port);
    connect(this,&QTcpServer::newConnection,this,&MachinePool::newClient);
}

Machine * MachinePool::at(int index)
{
    return _pool.at(index);
}

void MachinePool::newClient()
{
    auto * client = this->nextPendingConnection();
    this->_pool.append(new Machine(client));
}
