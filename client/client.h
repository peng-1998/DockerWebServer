#pragma once

#include "globalcommon.h"
#include "../tools/globalconfig.h"
#include "globaldata.h"
#include "globalevent.h"
#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
class Client final : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client() = default;

private:
    QTimer _timer;
    QSharedPointer<QTcpSocket> _socket;
};