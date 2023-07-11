#pragma once

#include "../global/globalconfig.h"
#include "../global/globaldata.h"
#include "globalcommon.h"
#include "globalevent.h"
#include <QHttpServer>
#include <QObject>
#include <QWebSocketServer>
#include <QSharedPointer>
class WebServer final : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);
    ~WebServer();

private:
    QSharedPointer<GlobalConfig> _config;
    QSharedPointer<GlobalData> _data;
    QSharedPointer<GlobalCommon> _common;
    QSharedPointer<GlobalEvent> _event;
    QSharedPointer<QHttpServer> _httpServer;

};
