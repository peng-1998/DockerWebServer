#include "webserver.h"
#include <QJsonDocument>
#include <QDebug>
#include <shadow.h>
#include <pwd.h>
#include <unistd.h>
#include <QCryptographicHash>
WebServer::WebServer(QObject *parent)
    : QObject{parent}
{
}

WebServer::WebServer(int port, QString loginPage, QString userPage,QString staticPath)
    : _loginPage(loginPage), _userPage(userPage),_staticPath(staticPath)
{
    _server.listen(QHostAddress::Any,port);
    _server.route("/",QHttpServerRequest::Method::Get,
                  [this](const QHttpServerRequest &request) {
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header,"Host");
        QString cookie = this->findHeaderItem(header,"Cookie");
        if(cookie=="failed"){
            resp.addHeader("location",QString("/static/"+this->_loginPage).toLocal8Bit());
            return resp;
        }
        QMap<QString,QString> cookieMap = this->getCookie(cookie);
        if(cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"]==this->user2md5(cookieMap["uname"]))
        {
            resp.addHeader("location",QString("/static/"+this->_userPage).toLocal8Bit());
        }
        else {
            resp.addHeader("location",QString("/static/"+this->_loginPage).toLocal8Bit());
        }
        return resp;
    });
    _server.route("/static/",QHttpServerRequest::Method::Get,
                  [this](QString file,const QHttpServerRequest &request){
        return QHttpServerResponse::fromFile(this->_staticPath+file);
    });
    _server.route("/static/images/",QHttpServerRequest::Method::Get,
                  [this](QString file,const QHttpServerRequest &request){
        return QHttpServerResponse::fromFile(this->_staticPath+"images/"+file);
    });
    _server.route("/login",QHttpServerRequest::Method::Post,
                  [this](const QHttpServerRequest &request){
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header,"Host").split(":")[0];
        QString body = request.body();
        auto items_str=body.split("&");
        auto items = this->toMap(items_str);
        auto account = items["account"];
        auto password = items["password"];
        auto holdlogin = items["holdlogin"];
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString hashcode = this->user2md5(account);
        if(this->testPasswd(account,password)){ //该功能需要以root/sudo启动程序,否则只能返回false
            QDateTime endDateTime = holdlogin=="True"?currentDateTime.addYears(3):currentDateTime.addSecs(60*60);
            resp.addHeader("location",QString("/").toLocal8Bit());
            resp.addHeader("Set-Cookie",QString("uname="+account+";Expires="+endDateTime.toString(Qt::RFC2822Date)+";Path=/").toUtf8());
            resp.addHeader("Set-Cookie",QString("hashcode="+hashcode+";Expires="+endDateTime.toString(Qt::RFC2822Date)+";Path=/").toUtf8());
            return resp;
        }
        return QHttpServerResponse("登陆失败,请重新尝试登陆或与管理员联系!",QHttpServerResponder::StatusCode::Continue);
    });
    _server.route("/login/validation",QHttpServerRequest::Method::Get,
                  [this](const QHttpServerRequest &request){
        QHttpServerResponse resp = QHttpServerResponse(QHttpServerResponder::StatusCode::Found);
        QList<QPair<QByteArray, QByteArray>> header = request.headers();
        QString host = this->findHeaderItem(header,"Host");
        QString cookie = this->findHeaderItem(header,"Cookie");
        if(cookie=="failed"){
            resp.addHeader("location",QString("/static/"+this->_loginPage).toLocal8Bit());
            return resp;
        }
        QMap<QString,QString> cookieMap = this->getCookie(cookie);
        if(cookieMap.contains("uname") && cookieMap.contains("hashcode") && cookieMap["hashcode"]==this->user2md5(cookieMap["uname"]))
        {
            return QHttpServerResponse("True");
        }
        else {
            resp.addHeader("location",QString("/static/"+this->_loginPage).toLocal8Bit());
        }
        return resp;
    });
}


QMap<QString,QString>  WebServer::getCookie(QString cookie)
{
    cookie=cookie.replace(" ","");
    auto cookiePair = cookie.split(";");
    return this->toMap(cookiePair);
}

QString  WebServer::findHeaderItem(QList<QPair<QByteArray, QByteArray> > & header, QString  key)
{
    for(const auto &[k, v] :header)
    {
        if(k==key)
        {
            return v;
        }
    }
    return "failed";
}

QMap<QString, QString> WebServer::toMap(QStringList & list)
{
    QMap<QString,QString> returnMap;
    for(auto kv:list)
    {
        auto _kv = kv.split("=");
        returnMap.insert(_kv[0],_kv[1]);
    }
    return returnMap;
}

bool WebServer::testPasswd(QString user, QString password)
{
    struct spwd *shd = ::getspnam(user.toStdString().c_str());//from /etc/shadow
    if (shd)
    {
        char * passwd = ::crypt(password.toStdString().c_str(), shd->sp_pwdp);
        if (passwd)
        {
            return std::strcmp(passwd, shd->sp_pwdp) == 0;
        }
    }
    return false;
}

QString WebServer::user2md5(QString uname)
{
    return QCryptographicHash::hash(QString("uname="+uname).toUtf8(),QCryptographicHash::Algorithm::Md5).toHex();
}








