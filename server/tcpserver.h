#pragma once

#include "globaldata.h"
#include <QObject>
#include <QTcpServer>
class TcpServer : public QTcpServer
{
public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer() = default;
}
