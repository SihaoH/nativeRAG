#pragma once
#include "TypeDefine.h"
#include <httplib.h>
#include <QString>
#include <QScopedPointer>

class ModelAdapter
{
public:
    static ModelAdapter* instance();
    
    void init(const QString& host, const QString& chat_m, const QString& embed_m);

    QString generate(const QString& prompt);
    QString generate(const QString& prompt, const QList<Reference>& refs, const QString& context);
    QString chat(const QString& messages);
    QList<float> embed(const QString& input);

private:
    ModelAdapter();

private:
    QString chatModel = "qwen2.5:7b";
    QString embedModel = "bge-m3";
    QScopedPointer<httplib::Client> ollama;
};
