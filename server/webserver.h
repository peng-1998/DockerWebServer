#pragma once

#include "../tools/globalconfig.h"
#include "../tools/jsonwebtoken/src/qjsonwebtoken.h"
#include "globalcommon.h"
#include "globaldata.h"
#include "globalevent.h"
#include "waitqueue.h"
#include <QHttpServer>
#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QWebSocket>
#include <QWebSocketServer>
class WebServer final : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);
    ~WebServer();

private:
    QSharedPointer<GlobalConfig> _config;
    QSharedPointer<GlobalData> _data;
    QSharedPointer<GlobalEvent> _event;
    QSharedPointer<QHttpServer> _httpServer;
    QSharedPointer<QWebSocketServer> _wsServer;
    QSharedPointer<QJsonWebToken> _jwt;
    QString _secret;

    // template <typename Func>
    // auto WebServer::jwtDecoratorArg(Func &&f)
    template <typename T>
    std::function<QHttpServerResponse(const QHttpServerRequest &)> jwtDecorator(T && t);
    template <typename T>
    std::function<QHttpServerResponse(QString, const QHttpServerRequest &)> jwtDecoratorArg(T && t);
};
