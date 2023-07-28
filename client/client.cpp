#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent)
{
    GlobalConfig::init("clientconfig.yaml");
    auto config = GlobalConfig::instance();
    _socket = QSharedPointer<QTcpSocket>::create(this);
    _socket->setProperty("host", QString::fromStdString((*config)["server"]["host"].as<std::string>()));
    _socket->setProperty("port", (*config)["server"]["port"].as<int>());
    connect(_socket.get(), &QTcpSocket::connected, GlobalEvent::instance().get(), &GlobalEvent::onConnected);
    connect(_socket.get(), &QTcpSocket::disconnected, GlobalEvent::instance().get(), &GlobalEvent::onDisconnected);
    connect(_socket.get(), &QTcpSocket::readyRead, GlobalEvent::instance().get(), &GlobalEvent::onReadyRead);
    _timer.callOnTimeout(GlobalEvent::instance().get(), &GlobalEvent::onSendHeartbeat);
    _timer.start(1000);
    _socket->connectToHost(_socket->property("host").toString(), _socket->property("port").toInt());
    auto data = GlobalData::instance();
    data->docker = QSharedPointer<DockerController>::create(this);
    data->gpu = QSharedPointer<NvidiaGPU>::create();
    data->logger = QSharedPointer<Logger>::create(QString::fromStdString((*config)["log"]["path"].as<std::string>()));
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) -> void
                           { GlobalData::instance()->logger->write(type, context, msg); });
}
