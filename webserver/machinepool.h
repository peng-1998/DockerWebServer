#ifndef MACHINEPOOL_H
#define MACHINEPOOL_H

#include <QTcpServer>
#include <QObject>

class MachinePool : public QTcpServer
{
public:
    class Machine
    {
    public:
        Machine() {};
    private:
        QTcpSocket * socket;
        QString _name;
    };
    MachinePool();
};

#endif // MACHINEPOOL_H
