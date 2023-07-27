#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QTcpSocket>
#include <QTimer>
#include "../tools/dockercontroller.h"
class GlobalData : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalData> instance();
    QSharedPointer<QTcpSocket> socket;
    QSharedPointer<DockerController> docker;

private:
    using QObject::QObject;
    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;
    GlobalData(GlobalData &&) = delete;
    static QSharedPointer<GlobalData> _instance;
};


