#include "Server.h"
#include "Logger.h"
#include "Dataset.h"
#include "ModelAdapter.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QThreadPool>

Server::Server(int argc, char* argv[])
    : QCoreApplication(argc, argv)
    , httpServer(new httplib::Server())
    , dataset(new Dataset(this))
{
}

Server::~Server()
{
    if (!httpServer.isNull()) {
        httpServer->stop();
    }
}

bool Server::init()
{
    // 读取配置文件
    QFile config_file("config.json");
    if (!config_file.open(QIODevice::ReadOnly)) {
        LOG(err) << "无法打开配置文件config.json";
        return false;
    }

    QJsonDocument config_doc = QJsonDocument::fromJson(config_file.readAll());
    if (!config_doc.isObject()) {
        LOG(err) << "配置文件config.json语法有误";
        return false;
    }
    QJsonObject config = config_doc.object();

    // 初始化ModelAdapter
    QJsonObject model_cfg = config["model"].toObject();
    ModelAdapter::instance()->init(
        model_cfg["host"].toString(),
        model_cfg["chat"].toString(),
        model_cfg["embed"].toString()
    );

    // 初始化Dataset
    //dataset->init(QJsonDocument(config["dataset"].toArray()).toJson());

    // 初始化http服务端
    initHttpServer(config["server"].toObject()["port"].toInt());

    return true;
}

int Server::run()
{
    if (!init()) {
        LOG(err) << "服务端初始化失败！";
        return -1;
    }

    LOG(info) << "服务端已启动...";
    return exec();
}

void Server::initHttpServer(int port)
{
    // 设置静态文件目录
    httpServer->set_mount_point("/", "./client");

    // API路由
    httpServer->Post("/api/chat", [this](const httplib::Request& req, httplib::Response& rsp) {
        onChat(req, rsp);
    });

    httpServer->Get("/api/dataset", [this](const httplib::Request&, httplib::Response& rsp) {
        QJsonArray datasets = QJsonArray::fromStringList(dataset->getList());
        rsp.set_content(QJsonDocument(datasets).toJson().toStdString(), "application/json");
    });

    QThreadPool::globalInstance()->start([=]() {
        LOG(info) << "HTTP服务端监听于0.0.0.0:" << QString::number(port);
        if (!httpServer->listen("0.0.0.0", port)) {
            LOG(err) << "HTTP服务端监听端口失败：" << QString::number(port);
            quit();
        }
    });
    QThread::msleep(500);
}

void Server::onChat(const httplib::Request& req, httplib::Response& rsp)
{
    auto json_doc = QJsonDocument::fromJson(QByteArray::fromStdString(req.body));
    auto req_body = json_doc.object();
    auto req_dataset = req_body["dataset"].toString();
    auto req_content = req_body["content"].toString();
    auto req_context = QJsonDocument(req_body["context"].toObject()).toJson();

    auto refs = dataset->search(req_content, req_dataset);
    auto reply = ModelAdapter::instance()->generate(req_content, refs, req_context);
    auto reply_obj = QJsonDocument::fromJson(reply.toUtf8()).object();
    QJsonArray reply_refs;
    for (const auto& ref : refs) {
        QJsonObject ref_obj;
        ref_obj["name"] = ref.name;
        ref_obj["file"] = ref.file;
        ref_obj["text"] = ref.text;
        reply_refs.append(ref_obj);
    }
    reply_obj["reference"] = reply_refs;
    auto reply_json = QJsonDocument(reply_obj).toJson();
    rsp.set_content(reply_json.toStdString(), "application/json");
}
