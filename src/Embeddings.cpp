#include "Embeddings.h"
#include "Logger.h"
#include "Database.h"
#include <faiss/IndexFlat.h>

Embeddings::Embeddings()
{
}

Embeddings* Embeddings::instance()
{
    static Embeddings embed;
    return &embed;
}

QList<long> Embeddings::search(const QList<float>& query_emb, const QString& name, int k)
{
    if (query_emb.size() != EMBEDDING_DIM) {
        LOG(err) << "查询向量维度不匹配";
        return {};
    }

    QList<float> distances(k);
    QList<faiss::idx_t> indices(k);

    // 执行相似度搜索
    auto faiss_index = loadFromDatabase(name);
    faiss_index->search(1, query_emb.data(), k, distances.data(), indices.data());
    QList<long> idx_ret;
    for (int i = 0; i < k; ++i) {
        if (indices[i] >= 0) {
            idx_ret.push_back(indices[i]);
        }
    }

    return idx_ret;
}

QSharedPointer<faiss::IndexIDMap> Embeddings::loadFromDatabase(const QString& name)
{
    QSharedPointer<faiss::IndexIDMap> index_map;
    try {
        auto baseIndex = new faiss::IndexFlatIP(EMBEDDING_DIM);
        index_map.reset(new faiss::IndexIDMap(baseIndex));
    } catch (const std::exception& e) {
        LOG(err) << "创建FAISS索引失败:" << e.what();
        return QSharedPointer<faiss::IndexIDMap>();
    }

    auto embed_map = Database::instance()->loadEmbeddings(name);
    for (auto i = embed_map.cbegin(), end = embed_map.cend(); i != end; ++i) {
        faiss::idx_t id = i.key();
        index_map->add_with_ids(1, i.value().data(), &id);
    }

    return index_map;
}
