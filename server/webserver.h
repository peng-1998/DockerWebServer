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

private:
    QHttpServer _httpServer;
    QWebSocketServer *_wsServer;
    QJsonWebToken _jwt;
    QString _secret;
    QTcpServer _tcpServer;

    // template <typename Func>
    // auto WebServer::jwtDecoratorArg(Func &&f)
    template <typename T>
    std::function<QHttpServerResponse(const QHttpServerRequest &)> jwtDecorator(T &&t);
    template <typename T>
    std::function<QHttpServerResponse(QString, const QHttpServerRequest &)> jwtDecoratorArg(T &&t);
};
