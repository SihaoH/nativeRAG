#include "Dataset.h"
#include "ModelAdapter.h"
#include "Database.h"
#include "Logger.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>

Dataset::Dataset(QObject* parent)
    : QObject(parent)
{
}

bool Dataset::loadConfig(const QString& json_path)
{
    QFile file(json_path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG(err) << "无法打开配置文件:" << json_path;
        return false;
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    if (!json_doc.isArray()) {
        LOG(err) << "配置文件格式错误，应为JSON数组";
        return false;
    }

    QJsonArray json_datasets = json_doc.array();
    for (const auto& dataset_item : json_datasets) {
        QJsonObject dataset_obj = dataset_item.toObject();
        QString dataset_name = dataset_obj["name"].toString();
        QString dataset_path = dataset_obj["path"].toString();
        
        if (dataset_name.isEmpty() || dataset_path.isEmpty()) {
            LOG(warn) << "数据集配置不完整，跳过:" << dataset_name;
            continue;
        }

        if (!processDataset(dataset_name, dataset_path)) {
            LOG(err) << "处理数据集失败:" << dataset_name;
            return false;
        }
    }

    return true;
}

bool Dataset::processDataset(const QString& name, const QString& dir_path)
{
    QDir dir(dir_path);
    if (!dir.exists()) {
        LOG(err) << "数据集目录不存在:" << dir_path;
        return false;
    }

    // 获取目录下所有文件
    QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : files) {
        if (!processFile(name, fileInfo.absoluteFilePath())) {
            LOG(err) << "处理文件失败:" << fileInfo.fileName();
            return false;
        }
    }

    return true;
}

bool Dataset::processFile(const QString& name, const QString& file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG(err) << "无法打开文件:" << file_path;
        return false;
    }

    QTextStream in(&file);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine();
        QString cleaned_text = cleanText(line);
        
        if (cleaned_text.isEmpty()) {
            continue;
        }

        // 获取文本的嵌入向量
        QList<float> text_embedding = ModelAdapter::instance()->embed(cleaned_text);
        if (text_embedding.isEmpty()) {
            LOG(err) << "获取文本嵌入失败:" << cleaned_text;
            return false;
        }

        // 保存到数据库
        Reference ref_data;
        ref_data.name = name;
        ref_data.file = file_path;
        ref_data.text = cleaned_text;
        
        long save_id = Database::instance()->saveEmbedding(ref_data, text_embedding);
        if (save_id < 0) {
            LOG(err) << "保存嵌入数据失败";
            return false;
        }
    }

    return true;
}

// TODO 清洗文本应该做更多
QString Dataset::cleanText(const QString& text)
{
    // 去除空白字符
    QString cleaned = text.trimmed().simplified();
    return cleaned;
} 