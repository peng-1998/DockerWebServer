#pragma once

#include "../global/globalconfig.h"
#include "globalcommon.h"
#include "jsonwebtoken/src/qjsonwebtoken.h"
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
class GlobalEvent : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalEvent> instance();

public:
    static QHttpServerResponse onHttpIndex(const QHttpServerRequest &request);
    static QHttpServerResponse onHttpWSServer(const QHttpServerRequest &request);
    static QHttpServerResponse onHttpWSClient(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAuthLogin(const QHttpServerRequest &request);
    static QHttpServerResponse onApiAuthRegister(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserSetProfile(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserSetPhoto(const QHttpServerRequest &request);
    static QHttpServerResponse onApiUserGetUser(const QString &account, const QHttpServerRequest &request);
    static QHttpServerResponse onApiMachinesInfo(const QHttpServerRequest &request);
signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
};
