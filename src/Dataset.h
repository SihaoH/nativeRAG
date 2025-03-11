#pragma once
#include <QObject>
#include <QString>
#include <QStringList>

class Dataset : public QObject
{
    Q_OBJECT
public:
    Dataset(QObject* parent = nullptr);
    ~Dataset() = default;

    // 从json文件加载数据集配置
    bool loadConfig(const QString& json_path = "dataset.json");

private:
    bool processDataset(const QString& name, const QString& dir_path);
    bool processFile(const QString& name, const QString& file_path);
    QString cleanText(const QString& text);
};
