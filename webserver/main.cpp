#include <QCoreApplication>
#include <QHttpServer>
#include "webserver.h"
#include <QSettings>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QSettings settings("settings.ini",QSettings::IniFormat);
    WebServer server(9999,settings.value("html/login").toString(),settings.value("html/userpage").toString(),settings.value("html/path").toString());
    return a.exec();
}