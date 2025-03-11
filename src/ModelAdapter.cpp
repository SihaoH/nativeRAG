#include "ModelAdapter.h"
#include "Logger.h"
#include <httplib.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ModelAdapter::ModelAdapter()
    : ollama(new httplib::Client("http://localhost:11434"))
{
}

ModelAdapter* ModelAdapter::instance()
{
    static ModelAdapter ollama;
    return &ollama;
}

QString ModelAdapter::generate(const QString& prompt)
{
    QJsonObject req_body;
    req_body["model"] = chatModel;
    req_body["prompt"] = prompt;
    req_body["stream"] = false;
    
    QJsonDocument req_doc(req_body);
    auto resp = ollama->Post("/api/generate", 
                            req_doc.toJson().toStdString(),
                            "application/json");
    
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(
            QByteArray::fromStdString(resp->body));
        return resp_doc.object()["response"].toString();
    }
    return "Error: Failed to generate response";
}

QString ModelAdapter::chat(const QString& messages)
{
    QJsonObject req_body;
    req_body["model"] = chatModel;
    req_body["messages"] = messages;
    req_body["stream"] = false;
    
    QJsonDocument req_doc(req_body);
    auto resp = ollama->Post("/api/chat",
                            req_doc.toJson().toStdString(),
                            "application/json");
    
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(
            QByteArray::fromStdString(resp->body));
        return resp_doc.object()["message"].toObject()["content"].toString();
    }
    return "Error: Failed to get chat response";
}

QList<float> ModelAdapter::embed(const QString& input)
{
    QJsonObject req_body;
    req_body["model"] = embedModel;
    req_body["input"] = input;
    
    QJsonDocument req_doc(req_body);
    auto resp = ollama->Post("/api/embed",
                            req_doc.toJson().toStdString(),
                            "application/json");
    
    QList<float> emb_vec;
    if (resp && resp->status == 200) {
        QJsonDocument resp_doc = QJsonDocument::fromJson(
            QByteArray::fromStdString(resp->body));
        QJsonArray emb_arr = resp_doc.object()["embeddings"].toArray().first().toArray();
        
        for (const auto& val : emb_arr) {
            emb_vec.append(val.toDouble());
        }
    }
    return emb_vec;
}
