#include "machine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QCoreApplication>
Machine::Machine(QObject *parent) :
    QObject{parent} {
}

QList<GPU> Machine::gpus() const {
    return _gpus;
}

QString Machine::name() const {
    return _name;
}

QList<QString> Machine::images() const {
    return _images;
}

QList<Container> Machine::containers() const {
    return _containers;
}

void Machine::refreshData() {
    this->_socket->write(_msgRefresh);
}

void Machine::creatContainer(QJsonObject &data) {
    QJsonObject msgobj{
        {"type", "create"},
        {"data", data}};
    this->_socket->write(QJsonDocument(msgobj).toJson(QJsonDocument::Compact));
}

void Machine::operationContainer(QString name, QString opt) {
    assert(opt == "start" || opt == "stop" || opt == "remove");
    QJsonObject msgobj{
        {"type", "operation"},
        {"data", QJsonObject{{"name", name}, {"opt", opt}}}};
    this->_socket->write(QJsonDocument(msgobj).toJson(QJsonDocument::Compact));
}

void Machine::exec(QString name, QString cmd) {
    QJsonObject msgobj{
        {"type", "exec"},
        {"data", QJsonObject{{"name", name}, {"cmd", cmd}}}};
    this->_socket->write(QJsonDocument(msgobj).toJson(QJsonDocument::Compact));
}

QString Machine::host() const {
    return _host;
}

bool Machine::verifyAccount(QString account, QString password) {
    QJsonObject msgobj{
        {"type", "verify"},
        {"data", QJsonObject{{"account", account}, {"password", password}}}};
    this->_socket->write(QJsonDocument(msgobj).toJson(QJsonDocument::Compact));
    QEventLoop loop;
    bool result;
    connect(this, &Machine::verifyAccountFinished, [&loop, &result](bool arg) {
        result = arg;
        loop.quit();
    });
    loop.exec();
    while (loop.isRunning()) {
        QCoreApplication::processEvents();
    }
    return result;
}

void Machine::readTransaction() {
    auto data = this->_socket->readAll();
    auto jsondata = QJsonDocument::fromJson(data);
    if (jsondata.isNull()) {
        emit this->giveup(this);
        return;
    }
    auto jsonobj = jsondata.object();
    if (jsonobj.contains("heartbeat")) {
        _lastHeartbeat = QDateTime::currentDateTime();
    }
    if (jsonobj.contains("gpus")) {
        auto jsonArray = jsonobj.value("gpus").toArray();
        this->_gpus.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            auto obj = jsonArray[i].toObject();
            this->_gpus.append(GPU(obj.value("type").toString(),
                                   obj.value("gpuid").toInt(),
                                   obj.value("max_memory").toDouble(),
                                   obj.value("used_memory").toDouble()));
        }
    }
    if (jsonobj.contains("images")) {
        auto jsonArray = jsonobj.value("images").toArray();
        this->_images.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            this->_images.append(jsonArray[i].toString());
        }
    }
    if (jsonobj.contains("containers")) {
        auto jsonArray = jsonobj.value("containers").toArray();
        this->_containers.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            auto obj = jsonArray[i].toObject();
            QList<QPair<int, int>> ports;
            auto jsPorts = obj.value("ports").toArray();
            for (int j = 0; j < jsPorts.size(); ++j) {
                auto port = jsPorts[i].toString();
                auto pp = port.split(":");
                ports.append(QPair<int, int>(pp[0].toInt(), pp[1].toInt()));
            }
            this->_containers.append(Container(obj.value("name").toString(),
                                               ports,
                                               obj.value("image").toString(),
                                               obj.value("stopped").toBool()));
        }
    }
    if (jsonobj.contains("name")) {
        this->_name = jsonobj.value("name").toString();
        emit this->initialized(this);
    }
    if (jsonobj.contains("verify")) {
        emit this->verifyAccountFinished(jsonobj.value("verify").toBool());
    }
}

Machine::Machine(QTcpSocket *socket) :
    _socket(socket) {
    _host = this->_socket->localAddress().toString();
    connect(this->_socket, &QIODevice::readyRead, this, &Machine::readTransaction);
    this->_socket->write(QJsonDocument(QJsonObject{{"type", "init"}}).toJson());
    connect(this->_socket, &QAbstractSocket::disconnected, this, [this]() {
        emit this->disconnected(this);
    });
}

Machine::~Machine() {
    delete this->_socket;
}
