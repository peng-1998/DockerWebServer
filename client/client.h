#pragma once

#include "globalcommon.h"
#include "../tools/globalconfig.hpp"
#include "../tools/gpuinfo.h"
#include "../tools/dockercontroller.h"
#include "globaldata.h"
#include "globalevent.h"
#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include "../tools/logger.hpp"
class Client final : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client() = default;

private:
    QTimer _timer;
    QTcpSocket _socket;
};