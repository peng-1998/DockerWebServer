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
    QHttpServerResponse onHttpIndex(const QHttpServerRequest &request);
    QHttpServerResponse onHttpWSServer(const QHttpServerRequest &request);
    QHttpServerResponse onHttpWSClient(const QHttpServerRequest &request);
    QHttpServerResponse onApiAuthLogin(const QHttpServerRequest &request);
    QHttpServerResponse onApiAuthRegister(const QHttpServerRequest &request);
    QHttpServerResponse onApiUserSetProfile(const QHttpServerRequest &request);
    QHttpServerResponse onApiUserSetPhoto(const QHttpServerRequest &request);
    QHttpServerResponse onApiUserGetUser(const QString &account, const QHttpServerRequest &request);
signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
};
