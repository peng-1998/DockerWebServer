#pragma once

#include "globalconfig.h"
#include "globaldata.h"
#include "globalcommon.h"
#include "globalevent.h"
#include "jsonwebtoken/src/qjsonwebtoken.h"
#include <QHttpServer>
#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QWebSocketServer>
#include <QWebSocket>
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

    template <typename T>
    std::function<QHttpServerResponse(const QHttpServerRequest &)> jwtDecorator(T t);
    template <typename T>
    std::function<QHttpServerResponse(const QString &, const QHttpServerRequest &)> jwtDecoratorArg(T t);
};

Q_DECLARE_METATYPE(QSharedPointer<WebServer>);
Q_DECLARE_METATYPE(QWeakPointer<WebServer>);
Q_DECLARE_METATYPE(QSharedPointer<QWebSocketServer>);
Q_DECLARE_METATYPE(QWeakPointer<QWebSocketServer>);
