#include "machinepool.h"

MachinePool::MachinePool(int port) {
    this->listen(QHostAddress::Any, port);
    connect(this, &QTcpServer::newConnection, this, &MachinePool::newClient);
}

QHash<QString, Machine *> MachinePool::pool() const {
    return _pool;
}

void MachinePool::newClient() {
    auto *machineSocket = this->nextPendingConnection();
    Machine *machine = new Machine(machineSocket);
    connect(machine, &Machine::giveup, this, [](Machine *m) {
        delete m;
    });
    connect(machine, &Machine::initialized, this, [this](Machine *m) {
        this->_pool.insert(m->name(), m);
        emit this->newMachine(m->name());
    });
    connect(machine, &Machine::disconnected, this, [this](Machine *m) {
        this->_pool.remove(m->name());
        emit this->delMachine(m->name());
        delete m;
    });
}
