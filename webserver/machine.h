#ifndef MACHINE_H
#define MACHINE_H

#include <QObject>
#include <QTcpSocket>
#include "gpu.h"
class Machine : public QObject
{
    Q_OBJECT
public:
    explicit Machine(QObject *parent = nullptr);

signals:

private:
    QTcpSocket * _socket;
    QString _name;
    QList<GPU> _gpus;
    QList<QString> _images;

};

#endif // MACHINE_H
