#include "webserver.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
WebServer::WebServer(QObject *parent) :
    QObject{parent} {
}

WebServer::WebServer(int port, QString loginPage, QString userPage, QString staticPath) :
    _loginPage(loginPage),
    _userPage(userPage),
    _staticPath(staticPath) {
    _server.listen(QHostAddress::Any, port);
    _server.route("/", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request) {
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header, "Host");
        QString cookie = this->findHeaderItem(header, "Cookie");
        if (cookie == "failed") {
            resp.addHeader("location", QString("/static/" + this->_loginPage).toLocal8Bit());
            return resp;
        }
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        if (cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"] == this->user2md5(cookieMap["uname"])) {
            resp.addHeader("location", QString("/static/" + this->_userPage).toLocal8Bit());
        } else {
            resp.addHeader("location", QString("/static/" + this->_loginPage).toLocal8Bit());
        }
        return resp;
    });
    _server.route("/static/", QHttpServerRequest::Method::Get, [this](QString file, const QHttpServerRequest &request) {
        return QHttpServerResponse::fromFile(this->_staticPath + file);
    });
    _server.route("/static/images/", QHttpServerRequest::Method::Get, [this](QString file, const QHttpServerRequest &request) {
        return QHttpServerResponse::fromFile(this->_staticPath + "images/" + file);
    });
    _server.route("/login", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header, "Host").split(":")[0];
        QString body = request.body();
        auto items_str = body.split("&");
        auto items = this->toMap(items_str);
        auto account = items["account"];
        auto password = items["password"];
        auto holdlogin = items["holdlogin"];
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString hashcode = this->user2md5(account);
        if (this->_database->hasUser(account) && this->_database->getPassword(account) == password) { // 该功能需要以root/sudo启动程序,否则只能返回false
            QDateTime endDateTime = holdlogin == "True" ? currentDateTime.addYears(3) : currentDateTime.addSecs(60 * 60);
            resp.addHeader("location", QString("/").toUtf8());
            resp.addHeader("Set-Cookie", QString("uname=" + account + ";Expires=" + endDateTime.toString(Qt::RFC2822Date) + ";Path=/").toUtf8());
            resp.addHeader("Set-Cookie", QString("hashcode=" + hashcode + ";Expires=" + endDateTime.toString(Qt::RFC2822Date) + ";Path=/").toUtf8());
            return resp;
        }
        return QHttpServerResponse("登陆失败,请重新尝试登陆或与管理员联系!", QHttpServerResponder::StatusCode::Continue);
    });
    _server.route("/login/validation", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request) {
        QHttpServerResponse resp =
            QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header, "Host");
        QString cookie = this->findHeaderItem(header, "Cookie");
        if (cookie == "failed") {
            resp.addHeader("location", QString("/static/" + this->_loginPage).toLocal8Bit());
            return resp;
        }
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        if (cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"] == this->user2md5(cookieMap["uname"])) {
            return QHttpServerResponse("True");
        } else {
            resp.addHeader("location", QString("/static/" + this->_loginPage).toUtf8());
        }
        return resp;
    });
    _server.route("/query/machines", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request) {
        if (_answersCache["machines"].lastupdate().addSecs(this->_answerLive) < QDateTime::currentDateTime()) {
            QJsonObject answer;
            for (const auto &name : this->_machinePool->pool().keys()) {
                const auto &mhptr = this->_machinePool->pool().value(name);
                answer.insert(name, mhptr->host());
            }
            _answersCache["machines"].setAnswer(
                QJsonDocument(answer).toJson());
        }
        return QHttpServerResponse(_answersCache["machines"].answer().toUtf8());
    });
    _server.route("/query/gpuinfo/", QHttpServerRequest::Method::Get, [this](QString machine, const QHttpServerRequest &request) {
        if (_answersCache["gpuinfo/" + machine].lastupdate().addSecs(this->_answerLive) < QDateTime::currentDateTime()) {
            if (this->_machinePool->pool().contains(machine)) {
                Machine *mhptr = this->_machinePool->pool().value(machine);
                QJsonArray answer;
                for (auto gpu : mhptr->gpus()) {
                    auto gpuinfo = gpu.toJson();
                    QString user = this->_gpuWaitQueue->currentUser(machine, gpu.gpuid());
                    gpuinfo.insert("user", user);
                    if (user == "") {
                        gpuinfo.insert("waittime", "");
                    } else {
                        int waittime = this->_gpuWaitQueue->waitTime(machine, gpu.gpuid());
                        QString waittime_str = QString::number(int(waittime / 3600)) + ":" + QString::number(int(waittime % 3600 / 60)) + ":" + QString::number(int(waittime % 60));
                        gpuinfo.insert("waittime", waittime_str);
                    }
                    answer.append(gpuinfo);
                }
                _answersCache["gpuinfo/" + machine].setAnswer(QJsonDocument(answer).toJson());
            } else {
                return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
            }
        }
        return QHttpServerResponse(
            _answersCache["gpuinfo/" + machine].answer().toUtf8());
    });
    _server.route("/register", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header, "Host").split(":")[0];
        QByteArray body = request.body();
        QJsonObject data = QJsonDocument::fromJson(body).object();
        QString account = data["account"].toString();
        QString password = data["password"].toString();
        QString email = data["email"].toString();
        this->_database->addUser(account, password, email);
        resp.addHeader("location", QString("/static/" + this->_loginPage).toUtf8());
    });
    _server.route("/query/userexist/", QHttpServerRequest::Method::Get, [this](QString account, const QHttpServerRequest &request) {
        if (this->_database->hasUser(account)) {
            return QHttpServerResponse("True");
        } else {
            return QHttpServerResponse("False");
        }
    });
    _server.route("/query/userinfo", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request) {
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString cookie = this->findHeaderItem(header, "Cookie");
        if (cookie == "failed") {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        if (cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"] == this->user2md5(cookieMap["uname"])) {
            QString account = cookieMap["uname"];
            if (this->_database->hasUser(account)) {
                QJsonObject userinfo;
                userinfo.insert("account", account);
                userinfo.insert("email", this->_database->getEmail(account));
                return QHttpServerResponse(QJsonDocument(userinfo).toJson());
            } else {
                return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
            }
        } else {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
    });
    _server.route("/user/setemail", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        if (this->checkIdentity(header) == false) {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        QString cookie = this->findHeaderItem(header, "Cookie");
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        QString uname = cookieMap["uname"];
        QByteArray body = request.body();
        QJsonObject data = QJsonDocument::fromJson(body).object();
        QString email = data["email"].toString();
        this->_database->setEmail(uname, email);
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
    });
    _server.route("/user/setpasswd", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        if (this->checkIdentity(header) == false) {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        QString cookie = this->findHeaderItem(header, "Cookie");
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        QString uname = cookieMap["uname"];
        QByteArray body = request.body();
        QJsonObject data = QJsonDocument::fromJson(body).object();
        QString passwd = data["passwd"].toString();
        QString oldpasswd = data["oldpasswd"].toString();
        if (this->_database->getPassword(uname) != oldpasswd) {
            return QHttpServerResponse("Old password is wrong");
        }
        this->_database->setPassword(uname, passwd);
        return QHttpServerResponse("Password changed");
    });
    _server.route("/user/addlinuxaccount", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        if (this->checkIdentity(header) == false) {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        QString cookie = this->findHeaderItem(header, "Cookie");
        QMap<QString, QString> cookieMap = this->getCookie(cookie);
        QString uname = cookieMap["uname"];
        QByteArray body = request.body();
        QJsonObject data = QJsonDocument::fromJson(body).object();
        QString machine = data["machine"].toString();
        QString account = data["account"].toString();
        QString password = data["password"].toString();
        if (this->_machinePool->pool().value(machine)->verifyAccount(account, password)) {
            this->_database->addLinuxAccount(uname, machine, account);
            return QHttpServerResponse("Account added");
        } else {
            return QHttpServerResponse("Account verify failed");
        }
    });
}

QMap<QString, QString> WebServer::getCookie(QString cookie) {
    cookie = cookie.replace(" ", "");
    auto cookiePair = cookie.split(";");
    return this->toMap(cookiePair);
}

QString WebServer::findHeaderItem(QList<QPair<QByteArray, QByteArray>> &header, QString key) {
    for (const auto &[k, v] : header) {
        if (k == key) {
            return v;
        }
    }
    return "failed";
}

QMap<QString, QString> WebServer::toMap(QStringList &list) {
    QMap<QString, QString> returnMap;
    for (auto kv : list) {
        auto _kv = kv.split("=");
        returnMap.insert(_kv[0], _kv[1]);
    }
    return returnMap;
}

bool WebServer::checkIdentity(QList<QPair<QByteArray, QByteArray>> header) {
    QString cookie = this->findHeaderItem(header, "Cookie");
    if (cookie == "failed") {
        return false;
    }
    QMap<QString, QString> cookieMap = this->getCookie(cookie);
    if (cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"] == this->user2md5(cookieMap["uname"]))
        return true;
    return false;
}

QString WebServer::user2md5(QString uname) {
    return QCryptographicHash::hash(QString("uname=" + uname).toUtf8(), QCryptographicHash::Algorithm::Md5).toHex();
}
