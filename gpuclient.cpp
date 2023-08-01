#include <QCoreApplication>
#include "client/client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client client(qApp);
    return a.exec();
}
