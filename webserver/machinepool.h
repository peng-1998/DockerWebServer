#ifndef MACHINEPOOL_H
#define MACHINEPOOL_H

#include <QTcpServer>
#include <QObject>
#include <QList>
#include "machine.h"
class MachinePool : public QTcpServer {
public:
    MachinePool(int port);

    QHash<QString, Machine *> pool() const;

signals:
    void newMachine(QString machine);
    void delMachine(QString machine);

private:
    QHash<QString, Machine *> _pool;
private slots:
    void newClient();
};

#endif // MACHINEPOOL_H
