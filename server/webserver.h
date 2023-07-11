#pragma once

#include <QObject>
#include "../global/globalconfig.h"
#include "../global/globaldata.h"
#include "globalcommon.h"
#include "globalevent.h"
#include <QHttpServer>
class WebServer : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);

signals:

public slots:

private:
    GlobalConfig * _config;
    GlobalData * _data;
    GlobalCommon * _common;
    GlobalEvent * _event;
    QHttpServer * _server;

};

