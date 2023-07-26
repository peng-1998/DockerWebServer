#pragma once

#include "dockerconnector.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class DockerController : public DockerConnector
{
public:
    explicit DockerController(QObject *parent = nullptr);

    struct Container
    {
        QString id;
        QString image;
        QString name;
        QList<QPair<int, int>> ports;
        bool running;
        Container(QJsonObject data)
        {
            id      = data["Id"].toString();
            name    = data["Name"].toString().remove(0, 1);
            image   = data["Image"].toString();
            running = data["State"].toString() == "running";
            QSet<QPair<int, int>> ports_set;
            auto ports_data = data["Ports"].toArray();
            for (auto port_data : ports_data)
            {
                auto port = port_data.toObject();
                ports_set << QPair<int, int>(port["PrivatePort"].toInt(), port["PublicPort"].toInt());
            }
            for (auto port : ports_set)
                ports << port;
        };
    };

    struct Image
    {
        QString name;
        QString id;
        Image(QJsonObject data)
        {
            name = data["RepoTags"].toArray()[0].toString();
            id   = data["Id"].toString();
        };
    };

    enum ContainerOpt
    {
        START   = 0,
        STOP    = 1,
        RESTART = 2
    };

    typedef QList<Container> Containers;
    typedef QList<Image> Images;

    Containers containers();
    Images images();
    Container container(const QString &name);
    Image image(const QString &name);

    QString createContainer(const QString &image, const QString &name, const QString &command, const QList<QPair<int, int>> &ports);
    std::optional<QString> buildImage(const QString &dockerfile, const QString &name);
    std::optional<QString> pushImage(const QString &name);
    std::optional<QString> pullImage(const QString &name);
    void removeContainer(const QString &name);
    void removeImage(const QString &name);
    void containerOpt(const QString &name, const ContainerOpt opt);

    QJsonObject _defualtContainerCreateData;

private:
    QString _dockerfilePath;
};
