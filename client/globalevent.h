#pragma once

#include <QHash>
#include <QObject>
class GlobalEvent : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<GlobalEvent> instance();

public slots:
    void onSendHeartbeat();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void messageHandler(const QJsonObject json);
    void messageHandleImage(const QJsonObject json);
    void messageHandleContainer(const QJsonObject json);
private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
    QHash<QString, std::function<void(const GlobalEvent &, const QJsonObject &)>> _messageHandlers;
};
