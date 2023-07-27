#pragma once



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
private:
    explicit GlobalEvent(QObject *parent = nullptr);
    GlobalEvent(const GlobalEvent &) = delete;
    GlobalEvent &operator=(const GlobalEvent &) = delete;
    GlobalEvent(GlobalEvent &&) = delete;
    static QSharedPointer<GlobalEvent> _instance;
};
