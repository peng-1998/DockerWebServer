#ifndef MACHINEPOOL_H
#define MACHINEPOOL_H

#include <QTcpServer>
#include <QObject>
#include <QList>
#include "machine.h"
class MachinePool : public QTcpServer
{
public:
    MachinePool(int port);
    Machine * at(int);
private:
    QList<Machine*> _pool;
private slots:
    void newClient();
};

#endif // MACHINEPOOL_H
