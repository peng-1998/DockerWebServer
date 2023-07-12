#include "globalinit.h"
#include <QWeakPointer>
#include "webserver.h"

QSharedPointer<GlobalInit> GlobalInit::_instance = QSharedPointer<GlobalInit>::create();

QSharedPointer<GlobalInit> GlobalInit::instance()
{
    return _instance;
}

void GlobalInit::initConfig()
{
    auto config = GlobalConfig::instance();
    if(config->contains("JWT/secret"))
        config->setValue("JWT/secret","12345678");
    if(config->contains("JWT/algorithm"))
        config->setValue("JWT/algorithm","HS256");
    if(config->contains("JWT/iss"))
        config->setValue("JWT/iss","admin");
    if(config->contains("Http/port"))
        config->setValue("Http/port",80);
    if(config->contains("WebSocket/port"))
        config->setValue("WebSocket/port",8080);
    if(config->contains("defaultPhoto"))
        config->setValue("defaultPhoto","/static/defaultPhoto/1.png");
    if(config->contains("defultPhotoPath"))
        config->setValue("defultPhotoPath","/static/defaultPhoto");
    if(config->contains("customPhotoPath"))
        config->setValue("customPhotoPath","/static/customPhoto");
}

GlobalInit::GlobalInit(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<QWeakPointer<WebServer>>("QWeakPointer<WebServer>");
    qRegisterMetaType<QSharedPointer<WebServer>>("QSharedPointer<WebServer>");

}
