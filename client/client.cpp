#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent),_socket(this), _timer(this)
{
    auto &config = GlobalConfig::instance();
    config.init("config.yaml");
    _socket.setProperty("host", QString::fromStdString(config["server"]["host"].as<std::string>()));
    _socket.setProperty("port", config["server"]["port"].as<int>());
    connect(&_socket, &QTcpSocket::connected, &GlobalEvent::instance(), &GlobalEvent::onConnected);
    connect(&_socket, &QTcpSocket::disconnected, &GlobalEvent::instance(), &GlobalEvent::onDisconnected);
    connect(&_socket, &QTcpSocket::readyRead, &GlobalEvent::instance(), &GlobalEvent::onReadyRead);
    _timer.callOnTimeout(&GlobalEvent::instance(), &GlobalEvent::onSendHeartbeat);
    _timer.start(1000);
    _socket.connectToHost(_socket.property("host").toString(), _socket.property("port").toInt());
    auto &data = GlobalData::instance();
    data.gpu = new NvidiaGPU();
    data.logger = new Logger(QString::fromStdString(config["log"]["path"].as<std::string>()));
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) -> void
                           { GlobalData::instance().logger->write(type, context, msg); });
}
