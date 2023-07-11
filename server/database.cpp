#include "database.h"
#include "../global/globalconfig.h"
#include <QFile>
#include <QtConcurrent>

QSharedPointer<DataBase> DataBase::_instance{nullptr};
QStringList DataBase::_user_column{"id", "account", "nickname", "password", "email", "phone", "photo"};
QStringList DataBase::_image_column{"id", "showname", "imagename", "init_args", "description"};
QStringList DataBase::_container_column{"id", "imageid", "userid", "showname", "containername", "portlist", "running"};
QStringList DataBase::_machine_column{"id", "ip", "gpu", "cpu", "memory", "online"};

QSharedPointer<DataBase> DataBase::instance()
{
    if (_instance == nullptr)
        _instance = QSharedPointer<DataBase>::create();
    return _instance;
}

void DataBase::insertUser(const QString &account, const QString &password, QString nickname, QString email, QString phone, QString photo)
{
    _query.prepare("INSERT INTO user (account, password, nickname, email, phone, photo) "
                   "VALUES (:account, :password, :nickname, :email, :phone, :photo)");
    _query.bindValue(":account", account);
    _query.bindValue(":password", password);
    _query.bindValue(":nickname", nickname);
    _query.bindValue(":email", email);
    _query.bindValue(":phone", phone);
    _query.bindValue(":photo", photo);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getUser(const int id, const std::optional<QStringList> &select)
{

    _query.prepare("SELECT :keys FROM user WHERE id = :id");
    _query.bindValue(":keys", select.value_or(_user_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_user_column))
        {
            result.insert(key, _query.value(key));
        }
        return result;
    }
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getUser(const QString &account, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM user WHERE account = :account");
    _query.bindValue(":keys", select.value_or(_user_column).join(","));
    _query.bindValue(":account", account);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_user_column))
        {
            result.insert(key, _query.value(key));
        }
        return result;
    }
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getUser(QList<QPair<QString, QVariant>> &where, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM user WHERE :where");
    _query.bindValue(":keys", select.value_or(_user_column).join(","));
    _query.bindValue(":where", QtConcurrent::blockingMappedReduced(
                                   where, [](const QPair<QString, QVariant> &pair)
                                   { return pair.first + " = :" + pair.second.toString(); },
                                   [](const QString &a, const QString &b)
                                   { return a + " AND " + b; }));
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_user_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getUserAll(const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM user");
    _query.bindValue(":keys", select.value_or(_user_column).join(","));
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_user_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
    return result;
}

void DataBase::updateUser(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE user SET :set WHERE id = :id");
    _query.bindValue(":set", QtConcurrent::blockingMappedReduced(
                                 set, [](const QPair<QString, QVariant> &pair)
                                 { return pair.first + " = :" + pair.second.toString(); },
                                 [](const QString &a, const QString &b)
                                 { return a + ", " + b; }));
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::updateUser(const QString &account, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE user SET :set WHERE account = :account");
    _query.bindValue(":set", QtConcurrent::blockingMappedReduced(
                                 set, [](const QPair<QString, QVariant> &pair)
                                 { return pair.first + " = :" + pair.second.toString(); },
                                 [](const QString &a, const QString &b)
                                 { return a + ", " + b; }));
    _query.bindValue(":account", account);
    _query.exec();
}

void DataBase::deleteUser(const int id)
{
    _query.prepare("DELETE FROM user WHERE id = :id");
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::deleteUser(const QString &account)
{
    _query.prepare("DELETE FROM user WHERE account = :account");
    _query.bindValue(":account", account);
    _query.exec();
}

bool DataBase::containsUser(const QString &account)
{
    _query.prepare("SELECT id FROM user WHERE account = :account");
    _query.bindValue(":account", account);
    _query.exec();
    return _query.next();
}

void DataBase::insertImage(const QString &imagename, const QString &showname, const QJsonObject &init_args, const QString &description)
{
    _query.prepare("INSERT INTO image (imagename, showname, init_args, description) "
                   "VALUES (:imagename, :showname, :init_args, :description)");
    _query.bindValue(":imagename", imagename);
    _query.bindValue(":showname", showname);
    _query.bindValue(":init_args", QString(QJsonDocument(init_args).toJson()));
    _query.bindValue(":description", description);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getImage(const int id, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM image WHERE id = :id");
    _query.bindValue(":keys", select.value_or(_image_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_image_column))
        {
            if (key == "init_args")
            {
                result.insert(key, QJsonDocument::fromJson(_query.value(key).toByteArray()).object());
            }
            else
            {
                result.insert(key, _query.value(key));
            }
        }
        return result;
    }
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getImage(const QString &imagename, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM image WHERE imagename = :imagename");
    _query.bindValue(":keys", select.value_or(_image_column).join(","));
    _query.bindValue(":imagename", imagename);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_image_column))
        {
            if (key == "init_args")
            {
                result.insert(key, QJsonDocument::fromJson(_query.value(key).toByteArray()).object());
            }
            else
            {
                result.insert(key, _query.value(key));
            }
        }
        return result;
    }
}

QList<QHash<QString, QVariant>> DataBase::getImageAll(const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM image");
    _query.bindValue(":keys", select.value_or(_image_column).join(","));
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_image_column))
        {
            if (key == "init_args")
            {
                tmp.insert(key, QJsonDocument::fromJson(_query.value(key).toByteArray()).object());
            }
            else
            {
                tmp.insert(key, _query.value(key));
            }
        }
        result.append(tmp);
    }
    return result;
}

void DataBase::updateImage(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE image SET :set WHERE id = :id");
    _query.bindValue(":set", QtConcurrent::blockingMappedReduced(
                                 set, [](const QPair<QString, QVariant> &pair)
                                 { return pair.first + " = " + (pair.first == "init_args" ? QString(pair.second.toJsonDocument().toJson(QJsonDocument::JsonFormat::Compact)) : pair.second.toString()); },
                                 [](const QString &a, const QString &b)
                                 { return a + ", " + b; }));
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::updateImage(const QString &imagename, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE image SET :set WHERE imagename = :imagename");
    _query.bindValue(":set", QtConcurrent::blockingMappedReduced(
                                 set, [](const QPair<QString, QVariant> &pair)
                                 { return pair.first + " = " + (pair.first == "init_args" ? QString(pair.second.toJsonDocument().toJson(QJsonDocument::JsonFormat::Compact)) : pair.second.toString()); },
                                 [](const QString &a, const QString &b)
                                 { return a + ", " + b; }));
    _query.bindValue(":imagename", imagename);
    _query.exec();
}

void DataBase::deleteImage(const int id)
{
    _query.prepare("DELETE FROM image WHERE id = :id");
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::deleteImage(const QString &imagename)
{
    _query.prepare("DELETE FROM image WHERE imagename = :imagename");
    _query.bindValue(":imagename", imagename);
    _query.exec();
}

bool DataBase::containsImage(const QString &imagename)
{
    _query.prepare("SELECT id FROM image WHERE imagename = :imagename");
    _query.bindValue(":imagename", imagename);
    _query.exec();
    return _query.next();
}

void DataBase::insertContainer(const QString &containername, const QString &showname, const int imageid, const int userid, const QJsonArray portlist, const bool running)
{
    _query.prepare("INSERT INTO container (containername, showname, imageid, userid, portlist, running) "
                   "VALUES (:containername, :showname, :imageid, :userid, :portlist, :running)");
    _query.bindValue(":containername", containername);
    _query.bindValue(":showname", showname);
    _query.bindValue(":imageid", imageid);
    _query.bindValue(":userid", userid);
    _query.bindValue(":portlist", QJsonDocument(portlist).toJson());
    _query.bindValue(":running", running);
    _query.exec();
}

void DataBase::insertContainer(const QString &containername, const QString &showname, const QString &imagename, const QString &account, const QJsonArray portlist, const bool running)
{
    _query.prepare("INSERT INTO container (containername, showname, imageid, userid, portlist, running) "
                   "VALUES (:containername, :showname, (SELECT id FROM image WHERE imagename = :imagename), (SELECT id FROM user WHERE account = :account), :portlist, :running)");
    _query.bindValue(":containername", containername);
    _query.bindValue(":showname", showname);
    _query.bindValue(":imagename", imagename);
    _query.bindValue(":account", account);
    _query.bindValue(":portlist", QJsonDocument(portlist).toJson());
    _query.bindValue(":running", running);
    _query.exec();
}

std::optional<QHash<QString, QVariant>> DataBase::getContainer(const int id, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE id = :id");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_container_column))
        {
            result.insert(key, _query.value(key));
        }
        return result;
    }
    return std::nullopt;
}

std::optional<QHash<QString, QVariant>> DataBase::getContainer(const QString &containername, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE containername = :containername");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":containername", containername);
    _query.exec();
    QHash<QString, QVariant> result;
    if (_query.next())
    {
        for (auto &key : select.value_or(_container_column))
        {
            result.insert(key, _query.value(key));
        }
        return result;
    }
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getContainerAll(const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_container_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
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
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_container_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
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
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_container_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerImage(const QString &image, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE imageid = (SELECT id FROM image WHERE imagename = :image)");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":image", image);
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_container_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
    return result;
}

QList<QHash<QString, QVariant>> DataBase::getContainerImage(const int id, const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM container WHERE imageid = :id");
    _query.bindValue(":keys", select.value_or(_container_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_container_column))
        {
            tmp.insert(key, _query.value(key));
        }
        result.append(tmp);
    }
    return result;
}

void DataBase::updateContainer(const int id, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE container SET :set WHERE id = :id");
    _query.bindValue(":set", QtConcurrent::blockingMapped(set, [](const QPair<QString, QVariant> &pair)
                                                          { return pair.first + " = " + pair.second.toString(); })
                                 .join(","));
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::updateContainer(const QString &containername, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE container SET :set WHERE containername = :containername");
    QStringList set_list;
    for (auto &pair : set)
    {
        set_list.append(pair.first + " = " + pair.second.toString());
    }
    _query.bindValue(":set", set_list.join(","));
    _query.bindValue(":containername", containername);
    _query.exec();
}

void DataBase::deleteContainer(const int id)
{
    _query.prepare("DELETE FROM container WHERE id = :id");
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::deleteContainer(const QString &containername)
{
    _query.prepare("DELETE FROM container WHERE containername = :containername");
    _query.bindValue(":containername", containername);
    _query.exec();
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
    _query.prepare("SELECT :keys FROM machine WHERE id = :id");
    _query.bindValue(":keys", select.value_or(_machine_column).join(","));
    _query.bindValue(":id", id);
    _query.exec();
    if (_query.next())
    {
        QHash<QString, QVariant> result;
        for (auto &key : select.value_or(_machine_column))
        {
            if (key == "gpu" || key == "cpu" || key == "memory" || key == "disk")
            {
                result.insert(key, QJsonDocument::fromJson(_query.value(key).toByteArray()).object());
            }
            else
            {
                result.insert(key, _query.value(key));
            }
        }
        return result;
    }
    return std::nullopt;
}

QList<QHash<QString, QVariant>> DataBase::getMachineAll(const std::optional<QStringList> &select)
{
    _query.prepare("SELECT :keys FROM machine");
    _query.bindValue(":keys", select.value_or(_machine_column).join(","));
    _query.exec();
    QList<QHash<QString, QVariant>> result;
    while (_query.next())
    {
        QHash<QString, QVariant> tmp;
        for (auto &key : select.value_or(_machine_column))
        {
            if (key == "gpu" || key == "cpu" || key == "memory" || key == "disk")
            {
                tmp.insert(key, QJsonDocument::fromJson(_query.value(key).toByteArray()).object());
            }
            else
            {
                tmp.insert(key, _query.value(key));
            }
        }
        result.append(tmp);
    }
}

void DataBase::updateMachine(const QString &id, const QList<QPair<QString, QVariant>> &set)
{
    _query.prepare("UPDATE machine SET :set WHERE id = :id");
    _query.bindValue(":set", QtConcurrent::blockingMapped(set, [](const QPair<QString, QVariant> &pair)
                                                          {
                                                            if(pair.first == "gpu" || pair.first == "cpu" || pair.first == "memory" || pair.first == "disk")
                                                                return pair.first + " = " + QString(pair.second.toJsonDocument().toJson(QJsonDocument::JsonFormat::Compact));
                                                            else
                                                                return pair.first + " = " + pair.second.toString(); })
                                 .join(","));
    _query.bindValue(":id", id);
    _query.exec();
}

void DataBase::deleteMachine(const QString &id)
{
    _query.prepare("DELETE FROM machine WHERE id = :id");
    _query.bindValue(":id", id);
    _query.exec();
}

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    QString dbPath = GlobalConfig::instance()->value("DataBase/db_file").toString();
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
                "email TEXT,"
                "phone TEXT,"
                "photo TEXT,"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS image ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "showname TEXT NOT NULL,"
                "imagename TEXT NOT NULL UNIQUE,"
                "init_args TEXT NOT NULL,"
                "description TEXT,"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS container ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "imageid INTEGER NOT NULL,"
                "userid INTEGER NOT NULL,"
                "showname TEXT NOT NULL,"
                "containername TEXT NOT NULL,"
                "portlist TEXT NOT NULL,"
                "running BOOLEAN NOT NULL,"
                ");");
    _query.exec("CREATE TABLE IF NOT EXISTS machine ("
                "id TEXT PRIMARY KEY,"
                "ip TEXT NOT NULL,"
                "gpu TEXT NOT NULL,"
                "cpu TEXT NOT NULL,"
                "disk TEXT NOT NULL,"
                "memory TEXT NOT NULL,"
                "online BOOLEAN NOT NULL,"
                ");");
}
