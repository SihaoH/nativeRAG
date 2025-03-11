#pragma once
#include <faiss/IndexIDMap.h>
#include <QSharedPointer>

class Embeddings
{
public:
    static Embeddings* instance();
    
    // 搜索最相似的文本
    QList<long> search(const QList<float>& query_embed, const QString& name = QString(), int k = 3);

private:
    Embeddings();
    ~Embeddings() = default;
    QSharedPointer<faiss::IndexIDMap> loadFromDatabase(const QString& name);

private:
    static constexpr int EMBEDDING_DIM = 1024;  // 根据实际的嵌入维度调整
};
