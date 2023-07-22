#include "dockercontroller.h"
#include <QDir>

DockerController::DockerController(QObject *parent)
    : DockerConnector{parent}
{
}

Containers DockerController::containers()
{
    auto [statusCode, headers, data] = get("/containers/json");
    Containers containers;
    for (auto container_data : data.toArray())
        containers << Container{container_data.toObject()};
    return containers;
}

Images DockerController::images()
{
    auto [statusCode, headers, data] = get("/containers/json");
    Images images;
    for (auto image_data : data.toArray())
        images << Image{image_data.toObject()};
    return images;
}

Container DockerController::container(const QString &name)
{
    auto [statusCode, headers, data] = get(QString("/containers/%1/json").arg(name));
    return Container{data.toObject()};
}

Image DockerController::image(const QString &name)
{
    auto [statusCode, headers, data] = get(QString("/images/%1/json").arg(name));
    return Image{data.toObject()};
}

QString DockerController::createContainer(const QString &image, const QString &name, const QString &command, const QList<QPair<int, int>> &ports)
{
    QString url = QString("/containers/create?name=%1").arg(name);
    Headers headers;
    headers << QPair<QByteArray, QByteArray>("Content-Type", "application/json");
    QJsonObject data = _defualtContainerCreateData;
    data["Image"] = image;
    if (!command.isEmpty())
        data["Cmd"] = QJsonArray::fromStringList(command.split(" "));
    if (!ports.isEmpty())
        for (auto port : ports)
            data["HostConfig"]["PortBindings"][QString("%1/tcp").arg(port.first)] = QJsonArray{QJsonObject{{"HostPort", port.second}}};
    auto [statusCode, headers, response] = post(url, headers, data);
    return response.toObject()["Id"].toString();
}

std::optional<QString> DockerController::buildImage(const QString &dockerfile, const QString &name)
{
    QDir dir{_dockerfilePath};
    dir.mkpath("build");
    QFile file{dir.absoluteFilePath("build/Dockerfile")};
    file.open(QIODevice::WriteOnly);
    file.write(dockerfile.toUtf8());
    file.close();
    QProcess process;
    process.run(QString("tar -czvf %1 -C %2 .").arg(dir.absoluteFilePath("build.tar.gz"), dir.absolutePath("build")));
    process.waitForFinished();
    process.close();
    QFile tarFile{dir.absoluteFilePath("build.tar.gz")};
    tarFile.open(QIODevice::ReadOnly);
    QByteArray tarData = tarFile.readAll();
    tarFile.close();
    QString url = QString("/build?t=%1").arg(name);
    Headers headers;
    headers << QPair<QByteArray, QByteArray>("Content-Type", "application/tar");
    auto [statusCode, headers, response] = post(url, headers, tarData);
    auto lastResponse = response.toArray().last().toObject();
    if (lastResponse.contains("errorDetail"))
        return std::nullopt;
    for (auto &line : response.toArray())
        if (line.toObject().contains("aux"))
            return line.toObject()["aux"]["ID"].toString();
}

std::optional<QString> DockerController::pushImage(const QString &name)
{
    auto splitStr = name.split(":");
    auto url      = QString("/images/%1/push?tag=%2").arg(splitStr[0], splitStr[1]);
    Headers headers;
    headers << QPair<QByteArray, QByteArray>("X-Registry-Auth", "{}");
    auto [statusCode, headers, response] = post(url, headers, QByteArray{});
    if (response.isObject())
        return std::nullopt;
    else
        for (auto &line : response.toArray())
            if (line.toObject().contains("aux"))
                return line.toObject()["aux"]["Digest"].toString();
}

std::optional<QString> DockerController::pullImage(const QString &name)
{
    auto splitStr                        = name.split(":");
    auto url                             = QString("/images/create?fromImage=%1&tag=%2").arg(splitStr[0], splitStr[1]);
    auto [statusCode, headers, response] = post(url, Headers{}, QByteArray{});
    if (response.isObject())
        return std::nullopt;
    else
        for (auto &line : response.toArray())
            if (line.toObject()["status"].toString().contains("Digest:"))
                return line.toObject()["status"].toString().split("Digest:")[1].trimmed();
}

void DockerController::removeContainer(const QString &name)
{
    delete_(QString("/containers/%1?force=true").arg(name));
}

void DockerController::removeImage(const QString &name)
{
    delete_(QString("/images/%1?force=true").arg(name));
}

void DockerController::containerOpt(const QString &name, const ContainerOpt opt)
{
    post(QString("/containers/%1/%2").arg(name, opt), Headers{}, QByteArray{});
}
