#include "dockerconnector.h"
#include <QCoreApplication>
#include <QEventLoop>
#include "tools.hpp"

Headers DockerConnector::empty_headers;
QByteArray DockerConnector::empty_data;
QJsonObject DockerConnector::empty_object;


Response DockerConnector::get(const QString &path)
{
    auto socket = connectDocker();
    QString request = QString("GET %1 HTTP/1.1\r\nHost: localhost\r\n\r\n").arg(path);
    socket->write(request.toUtf8());
    socket->waitForBytesWritten(100);
    socket->waitForDisconnected(1000);
    QString response = QString::fromUtf8(socket->readAll());
    return parseResponse(response);
}

Response DockerConnector::post(const QString &path, Headers &headers, const QJsonObject &data)
{
    auto       socket  = connectDocker();
    QByteArray request = formatRequest(path, Method::Post, headers, data);
    socket->write(request);
    socket->waitForBytesWritten(100);
    while (socket->isValid())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    QString response = QString::fromUtf8(socket->readAll());
    return parseResponse(response);
}

Response DockerConnector::delete_(const QString &path)
{
    auto       socket  = connectDocker();
    QByteArray request = formatRequest(path, Method::Delete, DockerConnector::empty_headers, DockerConnector::empty_data);
    socket->write(request);
    socket->waitForBytesWritten(100);
    while (socket->isValid())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    QString response = QString::fromUtf8(socket->readAll());
    return parseResponse(response);
}

Response DockerConnector::post(const QString &path, Headers &headers, const QByteArray &data)
{
    auto       socket  = connectDocker();
    QByteArray request = formatRequest(path, Method::Post, headers, data);
    socket->write(request);
    socket->waitForBytesWritten(100);
    await(!socket->isValid());
    QString response = QString::fromUtf8(socket->readAll());
    return parseResponse(response);
}

Response DockerConnector::parseResponse(const QString &data)
{
    QRegularExpression re("HTTP/\\d\\.\\d\\s+(\\d+)\\s+");
    QRegularExpressionMatch match       = re.match(data);
    int  statusCode  = match.captured(1).toInt();
    auto resp_split  = data.split("\r\n\r\n");
    auto header_str  = resp_split[0];
    auto header_list = header_str.split("\r\n");
    auto body_str    = resp_split[1];
    auto body_list   = body_str.split("\r\n");
    Headers headers;
    for (int i{0}; i < header_list.size(); i++)
    {
        auto header_split = header_list[i].split(":");
        headers << QPair<QByteArray, QByteArray>(header_split[0].toUtf8(), header_split[1].toUtf8());
    }
    if (body_list.size() == 1)
        return std::make_tuple(statusCode, headers, QJsonValue(QJsonDocument::fromJson(body_list[0].toUtf8()).object()));
    else
    {
        QJsonArray body;
        for (int i{0}; i < body_list.size(); i++)
        {
            if (Q_UNLIKELY(body_list[i].isEmpty()))
                continue;
            body << QJsonDocument::fromJson(body_list[i].toUtf8()).object();
        }
        return std::make_tuple(statusCode, headers, QJsonValue(body));
    }
}

QByteArray DockerConnector::formatRequest(const QString &path, const Method method, Headers &headers, const QJsonObject &data)
{
    QString request = QString("%1 %2 HTTP/1.1\r\nHost: localhost\r\n");
    if (method == Method::Post)
        request = request.arg("POST");
    else
        request = request.arg("GET");
    request = request.arg(path);
    request = std::transform_reduce(
        headers.begin(),
        headers.end(),
        request,
        [](QString a, QString b)
        { return a + b; },
        [](QPair<QByteArray, QByteArray> &header)
        { return QString("%1: %2\r\n").arg(header.first, header.second); });
    request += "\r\n";
    request += QJsonDocument(data).toJson(QJsonDocument::Compact);
    return request.toUtf8();
}

QByteArray DockerConnector::formatRequest(const QString &path, const Method method, Headers &headers, const QByteArray &data)
{
    QString request = QString("%1 %2 HTTP/1.1\r\nHost: localhost\r\n");
    if (method == Method::Post)
        request = request.arg("POST");
    else
        request = request.arg("GET");
    request = request.arg(path);
    request = std::transform_reduce(
        headers.begin(),
        headers.end(),
        request,
        [](QString a, QString b)
        { return a + b; },
        [](QPair<QByteArray, QByteArray> &header)
        { return QString("%1: %2\r\n").arg(header.first).arg(header.second); });
    request += "\r\n";
    return request.toUtf8() + data;
}

QSharedPointer<QLocalSocket> DockerConnector::connectDocker()
{
    auto socket = QSharedPointer<QLocalSocket>::create();
    socket->connectToServer("/var/run/docker.sock");
    // QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    socket->waitForConnected(100);
    return socket;
}