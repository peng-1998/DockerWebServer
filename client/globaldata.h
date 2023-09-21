#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include "../tools/dockercontroller.h"
#include "../tools/gpuinfo.h"
#include "../tools/logger.hpp"
class GlobalData : public QObject
{
    Q_OBJECT
public:
    static GlobalData& instance();
    QTcpSocket* socket;
    NvidiaGPU* gpu;
    Logger* logger;
private:
    using QObject::QObject;
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
};

