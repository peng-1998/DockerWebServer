#include "webserver.h"
#include <QSettings>
using Method = QHttpServerRequest::Method;
WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
    GlobalConfig::init("config.ini", QSettings::IniFormat);
    _config = GlobalConfig::instance();
    _common = GlobalCommon::instance();
    _data = GlobalData::instance();
    _event = GlobalEvent::instance();
    auto bind = [this](auto func)
    { return std::bind(func, this->_event, std::placeholders::_1); };
    _httpServer = QSharedPointer<QHttpServer>::create();
    _httpServer->listen(QHostAddress::Any, _config->value("Http/port").toInt());
    _httpServer->route("/", Method::Get, bind(&GlobalEvent::onHttpIndex));
    _httpServer->route("/ws/server", Method::Get, bind(&GlobalEvent::onHttpWSServer));
    _httpServer->route("/ws/client", Method::Get, bind(&GlobalEvent::onHttpWSClient));
    _httpServer->route("/api/auth/login", Method::Post, bind(&GlobalEvent::onApiAuthLogin));
    _httpServer->route("/api/auth/register", Method::Post, bind(&GlobalEvent::onApiAuthRegister));
}

WebServer::~WebServer()
{

}
