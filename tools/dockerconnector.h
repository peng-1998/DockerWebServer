#pragma once

#include <QByteArray>
#include <QHttpServerRequest>
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

typedef QList<QPair<QByteArray, QByteArray>> Headers;
typedef std::tuple<int, Headers, QJsonValue> Response;
using Method = QHttpServerRequest::Method;

class DockerConnector : public QObject
{
    Q_OBJECT
public:
    explicit DockerConnector(QObject *parent = nullptr);
    Response get(const QString &path);
    Response post(const QString &path, Headers &headers, const QJsonObject &data);
    Response post(const QString &path, Headers &headers, const QByteArray &data)
    Response delete_(const QString &path);
signals:

private:
    Response parseResponse(const QString &data);
    QByteArray formatRequest(const QString &path, const Method method, Headers &headers, const QJsonObject &data);
    QByteArray formatRequest(const QString &path, const Method method, Headers &headers, const QByteArray &data)
};
