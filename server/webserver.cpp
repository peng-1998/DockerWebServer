#include "webserver.h"
#include <QSettings>
using Method = QHttpServerRequest::Method;
WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    GlobalConfig::init("config.ini",QSettings::IniFormat);
    _config = GlobalConfig::instance();
    _common = GlobalCommon::instance();
    _data = GlobalData::instance();
    _event = GlobalEvent::instance();
    _server = new QHttpServer(this);
    _server->listen(QHostAddress::Any, _config->value("Http/port").toInt());
    _server->route("/",Method::Get,std::bind(&GlobalEvent::onHttpIndex,_event,, std::placeholders::_1, std::placeholders::_2));

}
