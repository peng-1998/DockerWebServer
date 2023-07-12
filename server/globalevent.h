#pragma once

#include <QObject>
#include <QHttpServerResponse>
#include <QHttpServerRequest>
#include "globalcommon.h"
#include "../global/globalconfig.h"
#include <QSharedPointer>
#include <QWeakPointer>
#include "jsonwebtoken/src/qjsonwebtoken.h"
class GlobalEvent : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalEvent> instance();

public:
    QHttpServerResponse  onHttpIndex(const QHttpServerRequest &request);
    QHttpServerResponse  onHttpWSServer(const QHttpServerRequest &request);
    QHttpServerResponse  onHttpWSClient(const QHttpServerRequest &request);
    QHttpServerResponse  onApiAuthLogin(const QHttpServerRequest &request);
    QHttpServerResponse  onApiAuthRegister(const QHttpServerRequest &request);
signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent&) = delete;
    GlobalEvent& operator=(const GlobalEvent&) = delete;
    GlobalEvent(GlobalEvent&&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
};
