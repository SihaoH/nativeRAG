#include "Dataset.h"
#include <QTest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QTextStream>

class TestDataset : public QObject
{
    Q_OBJECT

private:
    QString testDir;
    
private slots:
    void initTestCase()
    {
        // 创建测试目录
        testDir = QDir::currentPath() + "/test_dataset";
        QDir().mkpath(testDir);
        
        // 创建测试文件
        QFile file(testDir + "/test.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "这是第一段测试文本。\n\n";
            out << "这是第二段测试文本，包含一些特殊字符：!@#$%^&*()。\n\n";
            out << "This is English text for testing.\n\n";
            out << "这是最后一段测试文本。";
            file.close();
        }
    }
    
    void cleanupTestCase()
    {
        // 清理测试目录
        QDir dir(testDir);
        dir.removeRecursively();
    }

    void testInit()
    {
        Dataset dataset;
        
        // 创建测试配置
        QJsonArray config;
        QJsonObject ds;
        ds["name"] = "test_ds";
        ds["path"] = testDir;
        config.append(ds);
        
        // 初始化
        dataset.init(QJsonDocument(config).toJson());
        
        // 验证列表
        auto list = dataset.getList();
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0], QString("test_ds"));
    }

    void testGetList()
    {
        Dataset dataset;
        
        // 创建测试配置
        QJsonArray config;
        QJsonObject ds1, ds2;
        ds1["name"] = "test_ds1";
        ds1["path"] = testDir;
        ds2["name"] = "test_ds2";
        ds2["path"] = testDir;
        config.append(ds1);
        config.append(ds2);
        
        // 初始化并验证
        dataset.init(QJsonDocument(config).toJson());
        auto list = dataset.getList();
        QCOMPARE(list.size(), 2);
        QVERIFY(list.contains("test_ds1"));
        QVERIFY(list.contains("test_ds2"));
    }

    void testSearch()
    {
        Dataset dataset;
        
        // 创建测试配置
        QJsonArray config;
        QJsonObject ds;
        ds["name"] = "test_ds";
        ds["path"] = testDir;
        config.append(ds);
        
        // 初始化
        dataset.init(QJsonDocument(config).toJson());
        
        // 执行搜索
        auto results = dataset.search("测试文本", "test_ds", 3);
        // 由于依赖ModelAdapter和Embeddings,这里只验证返回类型
        QVERIFY(results.size() <= 3);
        
        if (!results.isEmpty()) {
            // 验证返回的引用是否包含测试文件的内容
            bool found = false;
            for (const auto& ref : results) {
                if (ref.file.contains("test.txt") && ref.text.contains("测试文本")) {
                    found = true;
                    break;
                }
            }
            QVERIFY(found);
        }
    }
};

QTEST_MAIN(TestDataset)
#include "TestDataset.moc" 