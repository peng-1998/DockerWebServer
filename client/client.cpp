#include "client.h"

Client::Client(QObject *parent)
{
    GlobalConfig::init("clientconfig.yaml");
    auto config = GlobalConfig::instance();
    _socket = QSharedPointer<QTcpSocket>(new QTcpSocket(this));
    _socket->setPrperty("host", (*config)["server"]["host"].as<std::string>());
    _socket->setPrperty("port", (*config)["server"]["port"].as<int>());
    connect(_socket.get(), &QTcpSocket::connected, GlobalEvent::instance().get(), &GlobalEvent::onConnected);
    connect(_socket.get(), &QTcpSocket::disconnected, GlobalEvent::instance().get(), &GlobalEvent::onDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead, GlobalEvent::instance().get(), &GlobalEvent::onReadyRead);
    _timer.callOnTimeout(GlobalEvent::instance().get(), &GlobalEvent::onSendHeartbeat)
    _timer.start(1000);
    _socket->connectToHost(_socket->property("host").toString(), _socket->property("port").toInt());
}
