#include <QObject>
#include <QTest>
#include <QFile>
#include "Database.h"

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // 删除可能存在的测试数据库文件
        QFile::remove("dataset.db");
    }

    void cleanupTestCase()
    {
        QFile::remove("dataset.db");
    }

    void testGetSavedTime()
    {
        auto db = Database::instance();
        
        // 测试不存在的文件
        auto time = db->getSavedTime("not_exist.txt");
        QVERIFY(!time.isValid());
        
        // 保存一条记录后测试
        Reference ref{"testGetSavedTime", "testGetSavedTime.txt", "testGetSavedTime content"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        db->saveEmbedding(ref, embedding);
        
        time = db->getSavedTime("testGetSavedTime.txt");
        QVERIFY(time.isValid());
    }

    void testRemoveDataset()
    {
        auto db = Database::instance();
        
        // 先插入测试数据
        Reference ref{"testRemoveDataset", "testRemoveDataset.txt", "testRemoveDataset content"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        db->saveEmbedding(ref, embedding);
        
        // 删除数据集
        db->removeDataset("testRemoveDataset");
        
        // 验证数据已被删除
        auto time = db->getSavedTime("testRemoveDataset.txt");
        QVERIFY(!time.isValid());
    }

    void testRemoveSection()
    {
        auto db = Database::instance();
        
        // 先插入测试数据
        Reference ref{"testRemoveSection", "testRemoveSection.txt", "testRemoveSection content"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        db->saveEmbedding(ref, embedding);
        
        // 删除文件记录
        db->removeSection("testRemoveSection.txt");
        
        // 验证数据已被删除
        auto time = db->getSavedTime("testRemoveSection.txt");
        QVERIFY(!time.isValid());
    }

    void testClearDataset()
    {
        auto db = Database::instance();
        
        // 先插入测试数据
        Reference ref1{"testClearDataset1", "testClearDataset1.txt", "testClearDataset content 1"};
        Reference ref2{"testClearDataset2", "testClearDataset2.txt", "testClearDataset content 2"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        db->saveEmbedding(ref1, embedding);
        db->saveEmbedding(ref2, embedding);
        
        // 清理不在列表中的数据集
        db->clearDataset({"testClearDataset1"});
        
        // 验证test_ds2已被删除
        auto time = db->getSavedTime("testClearDataset2.txt");
        QVERIFY(!time.isValid());
        
        // 验证test_ds1仍然存在
        time = db->getSavedTime("testClearDataset1.txt");
        QVERIFY(time.isValid());
    }

    void testClearSection()
    {
        auto db = Database::instance();
        
        // 先插入测试数据
        Reference ref1{"testClearSection", "testClearSection1.txt", "testClearSection content 1"};
        Reference ref2{"testClearSection", "testClearSection2.txt", "testClearSection content 2"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        db->saveEmbedding(ref1, embedding);
        db->saveEmbedding(ref2, embedding);
        
        // 清理不在列表中的文件
        db->clearSection("testClearSection", {"testClearSection1.txt"});
        
        // 验证test2.txt已被删除
        auto time = db->getSavedTime("testClearSection2.txt");
        QVERIFY(!time.isValid());
        
        // 验证test1.txt仍然存在
        time = db->getSavedTime("testClearSection1.txt");
        QVERIFY(time.isValid());
    }

    void testSaveAndLoadEmbedding()
    {
        auto db = Database::instance();
        
        // 保存测试数据
        Reference ref{"testSaveAndLoadEmbedding", "testSaveAndLoadEmbedding.txt", "testSaveAndLoadEmbedding content"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        long id = db->saveEmbedding(ref, embedding);
        QVERIFY(id > 0);
        
        // 加载并验证
        auto embeddings = db->loadEmbeddings("testSaveAndLoadEmbedding");
        QCOMPARE(embeddings.size(), 1);
        QCOMPARE(embeddings[id], embedding);
    }

    void testGetReferenceByIndices()
    {
        auto db = Database::instance();
        
        // 保存测试数据
        Reference ref{"test_ds", "test_ds.txt", "test content"};
        QList<float> embedding{0.1f, 0.2f, 0.3f};
        long id = db->saveEmbedding(ref, embedding);
        
        // 获取并验证
        auto refs = db->getReferenceByIndices({id});
        QCOMPARE(refs.size(), 1);
        QCOMPARE(refs[0].name, ref.name);
        QCOMPARE(refs[0].file, ref.file);
        QCOMPARE(refs[0].text, ref.text);
    }
};

QTEST_MAIN(TestDatabase)
#include "TestDatabase.moc" 