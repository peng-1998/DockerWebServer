#pragma once

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPair>
#include <QLocalSocket>
#include <QSharedPointer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QJsonValue>
#include <QHttpServerRequest>

typedef QList<QPair<QByteArray, QByteArray>> Headers;
typedef std::tuple<int, Headers, QJsonValue> Response;

using Method = QHttpServerRequest::Method;

class DockerConnector : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    Response get(const QString &path);
    Response post(const QString &path, Headers &headers, const QJsonObject &data);
    Response post(const QString &path, Headers &headers, const QByteArray &data);
    Response delete_(const QString &path);

    static Headers empty_headers;
    static QByteArray empty_data;
    static QJsonObject empty_object;

private:
    QSharedPointer<QLocalSocket> connectDocker();
    Response parseResponse(const QString &data);
    QByteArray formatRequest(const QString &path, const Method method, Headers &headers, const QJsonObject &data);
    QByteArray formatRequest(const QString &path, const Method method, Headers &headers, const QByteArray &data);
};
