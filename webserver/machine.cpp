#include "machine.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
Machine::Machine(QObject *parent)
    : QObject{parent}
{

}

QList<GPU> Machine::gpus() const
{
    return _gpus;
}

QString Machine::name() const
{
    return _name;
}

QList<QString> Machine::images() const
{
    return _images;
}

QList<Container> Machine::containers() const
{
    return _containers;
}

void Machine::refreshData()
{
    this->_socket->write(QString("{\"type\"=\"refresh\"}").toUtf8());
}

void Machine::creatContainer(QJsonObject & data)
{
    QString msg = "{\"type\"=\"create\",\"data\"="+QJsonDocument(data).toJson(QJsonDocument::Compact)+"}";
    this->_socket->write(msg.toUtf8());
}

void Machine::operationContainer(QString name, QString opt)
{
    assert(opt=="start"||opt=="stop"||opt=="remove");
    QString msg = "{\"type\"=\"create\",\"data\"={\"name\"=\""+name+"\",\"opt\"=\""+opt+"\"}}";
    this->_socket->write(msg.toUtf8());
}

void Machine::exec(QString name, QString cmd)
{
    QString msg = "{\"type\"=\"exec\",\"data\"={\"name\"=\""+name+"\",\"cmd\"=\""+cmd+"\"}}";
    this->_socket->write(msg.toUtf8());
}

void Machine::readTransaction()
{
    auto data = this->_socket->readAll();
    auto jsondata = QJsonDocument::fromJson(data).object();
    if(jsondata.contains("name")){
        this->_name = jsondata.value("name").toString();
    }
    if(jsondata.contains("gpus")){
        auto jsonArray = jsondata.value("gpus").toArray();
        this->_gpus.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            auto obj = jsonArray[i].toObject();
            this->_gpus.append(GPU(obj.value("type").toString(),
                                   obj.value("gpuid").toInt(),
                                   obj.value("max_memory").toDouble(),
                                   obj.value("used_memory").toDouble()));
        }
    }
    if(jsondata.contains("images")){
        auto jsonArray = jsondata.value("images").toArray();
        this->_images.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            this->_images.append(jsonArray[i].toString());
        }
    }
    if(jsondata.contains("containers")){
        auto jsonArray = jsondata.value("containers").toArray();
        this->_containers.clear();
        for (int i = 0; i < jsonArray.size(); i++) {
            auto obj = jsonArray[i].toObject();
            QList<QPair<int, int> > ports;
            auto jsPorts = obj.value("ports").toArray();
            for(int j=0;j<jsPorts.size();++j)
            {
                auto port = jsPorts[i].toString();
                auto pp = port.split(":");
                ports.append(QPair<int,int>(pp[0].toInt(),pp[1].toInt()));
            }
            this->_containers.append(Container(obj.value("name").toString(),
                                               ports,
                                               obj.value("image").toString(),
                                               obj.value("stopped").toBool()));
        }
    }
}

Machine::Machine(QTcpSocket *socket) : _socket(socket)
{
    _host=this->_socket->localAddress().toString();
    connect(this->_socket,&QIODevice::readyRead,this,&Machine::readTransaction);
    this->_socket->write(QString("{\"type\"=\"init\"}").toUtf8());
}
