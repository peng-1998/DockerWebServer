#include <QCoreApplication>
#include "server/webserver.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    WebServer server;
    return a.exec();
}
