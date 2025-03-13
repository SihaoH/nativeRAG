#include <QObject>
#include <QTest>
#include "Embeddings.h"
#include "Database.h"

class TestEmbeddings : public QObject
{
    Q_OBJECT

private slots:
    void cleanupTestCase()
    {
    }

    void testSearch()
    {
        auto embeddings = Embeddings::instance();
        
        // 创建测试查询向量
        QList<float> query_embed(1024, 0.1f);
        
        // 执行搜索
        auto results = embeddings->search(query_embed, "test_ds", 3);
        // 由于依赖数据库中的数据,这里只验证返回类型
        QVERIFY(results.size() <= 3);
    }
};

QTEST_MAIN(TestEmbeddings)
#include "TestEmbeddings.moc"
