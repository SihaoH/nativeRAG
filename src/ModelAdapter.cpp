#include "ModelAdapter.h"
#include "Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ModelAdapter::ModelAdapter()
{
}

ModelAdapter* ModelAdapter::instance()
{
    static ModelAdapter ollama;
    return &ollama;
}

void ModelAdapter::init(const QString& host, const QString& chat_m, const QString& embed_m)
{
    ollama.reset(new httplib::Client(host.toStdString()));
    chatModel = chat_m;
    embedModel = embed_m;
}

QString ModelAdapter::generate(const QString& prompt)
{
    QJsonObject req_body;
    req_body["model"] = chatModel;
    req_body["prompt"] = prompt;
    req_body["stream"] = false;
    auto resp = ollama->Post("/api/generate", 
                            QJsonDocument(req_body).toJson().toStdString(),
                            "application/json");
    
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(QByteArray::fromStdString(resp->body));
        return resp_doc.object()["response"].toString();
    }
    return "Error: Failed to generate response";
}

QString ModelAdapter::generate(const QString& prompt, const QList<Reference>& refs, const QString& context)
{
    QString ref_str;
    for (const auto& ref : refs) {
        ref_str.append(QString("- %1 \n").arg(ref.text));
    }
    QJsonObject req_body;
    req_body["model"] = chatModel;
    req_body["prompt"] = prompt;
    req_body["stream"] = false;
    req_body["context"] = QJsonDocument::fromJson(context.toUtf8()).array();
    req_body["system"] = QString(
"在<context></context>XML标签中使用以下上下文作为您学到的知识。\n"
"<context>\n%1\n</context>\n"
"当回答用户时：\n"
"-如果你不知道，就说你不知道。\n"
"-如果你不知道什么时候不确定，请要求澄清。\n"
"避免提及你是从上下文中获得信息的。\n"
"并根据用户问题的语言进行回答。\n").arg(ref_str);

    auto resp = ollama->Post("/api/generate", 
                            QJsonDocument(req_body).toJson().toStdString(),
                            "application/json");
    
    if (resp && resp->status == 200) {
        return QString::fromStdString(resp->body);
    }
    return "Error: Failed to generate response";
}

QString ModelAdapter::chat(const QString& messages)
{
    QJsonObject req_body;
    req_body["model"] = chatModel;
    req_body["messages"] = messages;
    req_body["stream"] = false;
    auto resp = ollama->Post("/api/chat",
                            QJsonDocument(req_body).toJson().toStdString(),
                            "application/json");
    
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(QByteArray::fromStdString(resp->body));
        return resp_doc.object()["message"].toObject()["content"].toString();
    }
    return "Error: Failed to get chat response";
}

QList<float> ModelAdapter::embed(const QString& input)
{
    QJsonObject req_body;
    req_body["model"] = embedModel;
    req_body["input"] = input;
    auto resp = ollama->Post("/api/embed",
                            QJsonDocument(req_body).toJson().toStdString(),
                            "application/json");
    
    QList<float> emb_vec;
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(QByteArray::fromStdString(resp->body));
        QJsonArray emb_arr = resp_doc.object()["embeddings"].toArray().first().toArray();
        
        for (const auto& val : emb_arr) {
            emb_vec.append(val.toDouble());
        }
    }
    return emb_vec;
}
