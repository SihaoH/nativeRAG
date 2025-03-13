#include <QObject>
#include <QTest>
#include "ModelAdapter.h"

class TestModelAdapter : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // 初始化ModelAdapter
        ModelAdapter::instance()->init(
            "http://localhost:11434",
            "qwen2.5:7b",
            "bge-m3"
        );
    }

    void cleanupTestCase()
    {
    }

    void testGenerate()
    {
        auto adapter = ModelAdapter::instance();
        
        // 测试基本生成
        QString response = adapter->generate("Hello");
        QVERIFY(!response.isEmpty());
        
        // 测试带引用的生成
        QList<Reference> refs = {
            {"test", "test.txt", "This is a test reference"}
        };
        response = adapter->generate("Hello", refs, "{}");
        QVERIFY(!response.isEmpty());
    }

    void testChat()
    {
        auto adapter = ModelAdapter::instance();
        
        QString response = adapter->chat(R"(
        [
            {"role": "user", "content": "Hello"}
        ])");
        QVERIFY(!response.isEmpty());
    }

    void testEmbed()
    {
        auto adapter = ModelAdapter::instance();
        
        auto embedding = adapter->embed("Test text for embedding");
        QVERIFY(!embedding.isEmpty());
        QCOMPARE(embedding.size(), 1024); // bge-m3的维度是1024
    }
};

QTEST_MAIN(TestModelAdapter)
#include "TestModelAdapter.moc" 