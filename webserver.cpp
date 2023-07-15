#include <QCoreApplication>
#include "server/webserver.h"
#include "server/globalinit.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    GlobalInit::init();
    WebServer server;
    return a.exec();
}
