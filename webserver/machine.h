#ifndef MACHINE_H
#define MACHINE_H

#include <QObject>
#include <QTcpSocket>
#include "gpu.h"
#include "container.h"
#include <QJsonObject>
class Machine : public QObject
{
    Q_OBJECT
public:
    explicit Machine(QObject *parent = nullptr);
    Machine(QTcpSocket *socket);

    QList<GPU> gpus() const;

    QString name() const;

    QList<QString> images() const;

    QList<Container> containers() const;

    void refreshData();

    void creatContainer(QJsonObject &);

    void operationContainer(QString,QString);

    void exec(QString,QString);

signals:

private:
    QTcpSocket * _socket;
    QString _host;
    QString _name;
    QList<GPU> _gpus;
    QList<QString> _images;
    QList<Container> _containers;

private slots:
    void readTransaction();
};

#endif // MACHINE_H
