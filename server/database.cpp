#include "database.h"
#include "../tools/globalconfig.h"
#include <QDebug>
#include <QFile>
#include <QSqlError>
#include <QSqlRecord>
#include <QtConcurrent>
#include <algorithm>

QStringList DataBase::_user_column{"id", "account", "nickname", "password", "salt", "email", "phone", "photo"};
QStringList DataBase::_image_column{"id", "showname", "imagename", "init_args", "description"};
QStringList DataBase::_container_column{"id", "imageid", "userid", "showname", "containername", "portlist", "running"};
QStringList DataBase::_machine_column{"id", "ip", "gpu", "cpu", "memory", "online"};
QStringList JsonKeys{"gpu", "cpu", "memory", "disk", "init_args", "portlist"};

DataBase& DataBase::instance()
{
    static DataBase _instance;
    return _instance;
}

void DataBase::insertUser(const QString &account, const QString &password, const QString &salt, const QString nickname, const QString email, const QString phone, const QString photo)
{
    _query.prepare("INSERT INTO user (account, password, salt, nickname, email, phone, photo) "
                   "VALUES (:account, :password, :salt, :nickname, :email, :phone, :photo)");
    _query.bindValue(":account", account);
    _query.bindValue(":password", password);
    _query.bindValue(":salt", salt);
    _query.bindValue(":nickname", nickname);
    _query.bindValue(":email", email);
    _query.bindValue(":phone", phone);
    _query.bindValue(":photo", photo);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getUser(const int id, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM user WHERE id = %2").arg(select.value_or(_user_column).join(", "), QString::number(id)));
    QHash<QString, QVariant> result;
    if (_query.next())
        return formatResult(_query, select.value_or(_user_column));
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getUser(const QString &account, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM user WHERE account = '%2'").arg(select.value_or(_user_column).join(", "), account));
    qDebug() << _query.result();
    QHash<QString, QVariant> result;
    if (_query.next())
        return formatResult(_query, select.value_or(_user_column));
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getUser(QList<QPair<QString, QVariant>> &where, const std::optional<QStringList> &select)
{
    auto sql = QString("SELECT %1 FROM user WHERE %2")
                   .arg(select.value_or(_user_column).join(", "))
                   .arg(formatWhere(where));
    _query.exec(sql);
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_user_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getUserAll(const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT {} FROM user").arg(select.value_or(_user_column).join(", ")));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_user_column)));
    return result;
}

void DataBase::updateUser(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE user SET %1 WHERE id = %2").arg(formatSet(set), id));
}

void DataBase::updateUser(const QString &account, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE user SET %1 WHERE account = '%2'").arg(formatSet(set), account));
}

void DataBase::deleteUser(const int id)
{
    _query.exec(QString("DELETE FROM user WHERE id = %1").arg(id));
}

void DataBase::deleteUser(const QString &account)
{
    _query.exec(QString("DELETE FROM user WHERE account = '%1'").arg(account));
}

bool DataBase::containsUser(const QString &account)
{
    _query.exec(QString("SELECT id FROM user WHERE account = '%1'").arg(account));
    return _query.next();
}

void DataBase::insertImage(const QString &imagename, const QString &showname, const QJsonObject &init_args, const QString &description)
{
    _query.prepare("INSERT INTO image (imagename, showname, init_args, description) "
                   "VALUES (:imagename, :showname, :init_args, :description)");
    _query.bindValue(":imagename", imagename);
    _query.bindValue(":showname", showname);
    _query.bindValue(":init_args", QString(QJsonDocument(init_args).toJson(QJsonDocument::Compact)));
    _query.bindValue(":description", description);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getImage(const int id, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM image WHERE id = %2").arg(select.value_or(_image_column).join(", "), QString::number(id)));
    QHash<QString, QVariant> result;
    if (_query.next())
        return formatResult(_query, select.value_or(_image_column));
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getImage(const QString &imagename, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM image WHERE imagename = '%2'").arg(select.value_or(_image_column).join(", "), imagename));
    if (_query.next())
        return formatResult(_query, select.value_or(_image_column));
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getImageAll(const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM image").arg(select.value_or(_image_column).join(", ")));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_image_column)));
    return result;
}

void DataBase::updateImage(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE image SET %1 WHERE id = %2").arg(formatSet(set), id));
}

void DataBase::updateImage(const QString &imagename, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE image SET %1 WHERE imagename = '%2'").arg(formatSet(set), imagename));
}

void DataBase::deleteImage(const int id)
{
    _query.exec(QString("DELETE FROM image WHERE id = %1").arg(id));
}

void DataBase::deleteImage(const QString &imagename)
{
    _query.exec(QString("DELETE FROM image WHERE imagename = '%1'").arg(imagename));
}

bool DataBase::containsImage(const QString &imagename)
{
    _query.exec(QString("SELECT id FROM image WHERE imagename = '%1'").arg(imagename));
    return _query.next();
}

void DataBase::insertContainer(const QString &containername, const QString &showname, const int imageid, const int userid, const QString &machineId, const QJsonArray portlist, const bool running)
{
    _query.prepare("INSERT INTO container (containername, showname, imageid, userid, machineid, portlist, running) "
                   "VALUES (:containername, :showname, :imageid, :userid, :machineid, :portlist, :running)");
    _query.bindValue(":containername", containername);
    _query.bindValue(":showname", showname);
    _query.bindValue(":imageid", imageid);
    _query.bindValue(":userid", userid);
    _query.bindValue(":machineid", machineId);
    _query.bindValue(":portlist", QJsonDocument(portlist).toJson());
    _query.bindValue(":running", running);
    _query.exec();
}

void DataBase::insertContainer(const QString &containername, const QString &showname, const QString &imagename, const QString &account, const QString &machineId, const QJsonArray portlist, const bool running)
{
    _query.prepare("INSERT INTO container (containername, showname, imageid, userid, machineid, portlist, running) "
                   "VALUES (:containername, :showname, (SELECT id FROM image WHERE imagename = :imagename), (SELECT id FROM user WHERE account = :account), :machineid, :portlist, :running)");
    _query.bindValue(":containername", containername);
    _query.bindValue(":showname", showname);
    _query.bindValue(":imagename", imagename);
    _query.bindValue(":account", account);
    _query.bindValue(":machineid", machineId);
    _query.bindValue(":portlist", QJsonDocument(portlist).toJson());
    _query.bindValue(":running", running);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getContainer(const int id, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM container WHERE id = %2").arg(select.value_or(_container_column).join(", "), QString::number(id)));
    if (_query.next())
        return formatResult(_query, select.value_or(_container_column));
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getContainer(const QString &containername, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM container WHERE containername = '%2'").arg(select.value_or(_container_column).join(", "), containername));
    if (_query.next())
        return formatResult(_query, select.value_or(_container_column));
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getContainerAll(const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM container").arg(select.value_or(_container_column).join(", ")));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerUser(const QString &account, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE userid = (SELECT id FROM user WHERE account = :account)");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":account", account);
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerUser(const int id, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE userid = :id");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerImage(const QString &image, const std::optional<QStringList> &select)
{
    auto sql = QString("SELECT %1 FROM container WHERE imageid = (SELECT id FROM image WHERE imagename = %2)")
                   .arg(select.value_or(_container_column).join(", "), image);
    _query.exec(sql);
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerImage(const int id, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM container WHERE imageid = %2").arg(select.value_or(_container_column).join(", "), QString::number(id)));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerMachine(const QString &machineId, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM container WHERE machineid = '%2'").arg(select.value_or(_container_column).join(", "), machineId));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_container_column)));
    return result;
}

void DataBase::updateContainer(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE container SET %1 WHERE id = %2").arg(formatSet(set), id));
}

void DataBase::updateContainer(const QString &containername, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE container SET %1 WHERE containername = '%2'").arg(formatSet(set), containername));
}

void DataBase::updateContainerRunning(const QString &containername, const bool running)
{
    _query.prepare("UPDATE container SET running = :running WHERE containername = :containername");
    _query.bindValue(":running", running);
    _query.bindValue(":containername", containername);
    _query.exec();
}

void DataBase::deleteContainer(const int id)
{
    _query.exec(QString("DELETE FROM container WHERE id = %1").arg(id));
}

void DataBase::deleteContainer(const QString &containername)
{
    _query.exec(QString("DELETE FROM container WHERE containername = '%1'").arg(containername));
}

bool DataBase::containsContainer(const QString &containername)
{
    _query.exec(QString("SELECT id FROM container WHERE containername = '%1'").arg(containername));
    return _query.next();
}

void DataBase::insertMachine(const QString &id, const QString &ip, const QJsonObject &gpu, const QJsonObject &cpu, const QJsonObject &memory, const QJsonObject &disk, const bool online)
{
    _query.prepare("INSERT INTO machine (id, ip, gpu, cpu, memory, disk, online) VALUES (:id, :ip, :gpu, :cpu, :memory, :disk, :online)");
    _query.bindValue(":id", id);
    _query.bindValue(":ip", ip);
    _query.bindValue(":gpu", QString(QJsonDocument(gpu).toJson(QJsonDocument::Compact)));
    _query.bindValue(":cpu", QString(QJsonDocument(cpu).toJson(QJsonDocument::Compact)));
    _query.bindValue(":memory", QString(QJsonDocument(memory).toJson(QJsonDocument::Compact)));
    _query.bindValue(":disk", QString(QJsonDocument(disk).toJson(QJsonDocument::Compact)));
    _query.bindValue(":online", online);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getMachine(const QString &id, const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM machine WHERE id = '%2'").arg(select.value_or(_machine_column).join(", "), id));
    if (_query.next())
        return formatResult(_query, select.value_or(_machine_column));
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getMachineAll(const std::optional<QStringList> &select)
{
    _query.exec(QString("SELECT %1 FROM machine").arg(select.value_or(_machine_column).join(", ")));
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
        result.append(formatResult(_query, select.value_or(_machine_column)));
    return result;
}

void DataBase::updateMachine(const QString &id, const QList<QPair<QString, QVariant>> &set)
{
    _query.exec(QString("UPDATE machine SET %1 WHERE id = '%2'").arg(formatSet(formatInput(set)), id));
}

void DataBase::deleteMachine(const QString &id)
{
    _query.exec(QString("DELETE FROM machine WHERE id = '%1'").arg(id));
}

bool DataBase::containsMachine(const QString &id)
{
    _query.exec(QString("SELECT id FROM machine WHERE id = '%1'").arg(id));
    return _query.next();
}

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    QString dbPath = QString::fromStdString(GlobalConfig::instance()["DataBase"]["db_file"].as<std::string>());
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(dbPath);
    _db.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_OPEN_READWRITE;QSQLITE_OPEN_CREATE;QSQLITE_OPEN_SHAREDCACHE");
    bool db_exists = QFile::exists(dbPath);
    _db.open();
    _query = QSqlQuery(_db);
    if (!db_exists)
    {
        creatTable();
    }
}

void DataBase::creatTable()
{
    _query.exec("CREATE TABLE IF NOT EXISTS user ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "account TEXT NOT NULL UNIQUE,"
                "nickname TEXT,"
                "password TEXT NOT NULL,"
                "salt TEXT NOT NULL,"
                "email TEXT,"
                "phone TEXT,"
                "photo TEXT"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS image ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "showname TEXT NOT NULL,"
                "imagename TEXT NOT NULL UNIQUE,"
                "init_args TEXT NOT NULL,"
                "description TEXT"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS container ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "imageid INTEGER NOT NULL,"
                "userid INTEGER NOT NULL,"
                "machineid TEXT NOT NULL,"
                "showname TEXT NOT NULL,"
                "containername TEXT NOT NULL,"
                "portlist TEXT NOT NULL,"
                "running BOOLEAN NOT NULL"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS machine ("
                "id TEXT PRIMARY KEY,"
                "ip TEXT NOT NULL,"
                "gpu TEXT NOT NULL,"
                "cpu TEXT NOT NULL,"
                "disk TEXT NOT NULL,"
                "memory TEXT NOT NULL,"
                "online BOOLEAN NOT NULL"
                ");");
    qDebug() << _query.lastError().text();
}

QString DataBase::formatWhere(const QList<QPair<QString, QVariant>> &where)
{
    return std::transform_reduce(
        where.begin(),
        where.end(),
        QString(""),
        [](const QString &a, const QString &b)
        { return a + " AND " + b; },
        [](const QPair<QString, QVariant> &pair)
        { return pair.first + " = " + pair.second.toString(); });
}

QString DataBase::formatSet(const QList<QPair<QString, QVariant>> &set)
{
    return std::transform_reduce(
        set.begin(),
        set.end(),
        QString(""),
        [](const QString &a, const QString &b)
        { return a + ", " + b; },
        [](const QPair<QString, QVariant> &pair)
        { return pair.first + " = " + pair.second.toString(); });
}

QHash<QString, QVariant> DataBase::formatResult(const QSqlQuery &query, const QStringList &select)
{
    QHash<QString, QVariant> result;
    for (auto &key : select)
    {
        auto value = query.value(key);
        if (JsonKeys.contains(key))
            result.insert(key, QJsonDocument::fromJson(value.toByteArray()).object());
        else
            result.insert(key, value);
    }
    return result;
}

QList<QPair<QString, QVariant>> DataBase::formatInput(const QList<QPair<QString, QVariant>> &input)
{
    QList<QPair<QString, QVariant>> result;
    for (auto &pair : input)
    {
        if (JsonKeys.contains(pair.first))
            result.append(QPair<QString, QVariant>(pair.first, QString(QJsonDocument::fromVariant(pair.second).toJson(QJsonDocument::Compact))));
        else
            result.append(pair);
    }
    return result;
}
