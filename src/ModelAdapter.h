#pragma once
#include <QString>

namespace httplib { class Client; }
class ModelAdapter
{
public:
    static ModelAdapter* instance();
    
    QString generate(const QString& prompt);
    QString chat(const QString& messages);
    QList<float> embed(const QString& input);

private:
    ModelAdapter();
    const QString chatModel = "qwen2.5:7b";
    const QString embedModel = "bge-m3";
    QScopedPointer<httplib::Client> ollama;
};
