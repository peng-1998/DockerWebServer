#pragma once

#include <QObject>
#include <QHttpServerResponse>
class GlobalEvent : public QObject
{
    Q_OBJECT
public:
    static GlobalEvent * instance();

public slots:
    QHttpServerResponse && onHttpIndex(const QHttpServerRequest &request,QHttpServerResponse && response);
signals:

private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent&) = delete;
    GlobalEvent& operator=(const GlobalEvent&) = delete;
    GlobalEvent(GlobalEvent&&) = delete;
    static GlobalEvent * _instance;
};
