#include "Dataset.h"
#include "ModelAdapter.h"
#include "Database.h"
#include "Logger.h"
#include "Embeddings.h"
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

void Dataset::init(const QString& json)
{
    auto json_doc = QJsonDocument::fromJson(json.toUtf8());
    if (!json_doc.isArray()) {
        LOG(err) << "配置文件格式错误，应为JSON数组";
        return;
    }

    auto json_datasets = json_doc.array();
    for (const auto& item : json_datasets) {
        QJsonObject item_obj = item.toObject();
        list.append(DatasetItem{item_obj["name"].toString(), item_obj["path"].toString()});
    }

    reload();
}

QStringList Dataset::getList() const
{
    QStringList name_list;
    for (const auto& ds : list) {
        name_list.append(ds.name);
    }
    return name_list;
}

QList<Reference> Dataset::search(const QString& query, const QString& name, int k)
{
    auto embed = ModelAdapter::instance()->embed(query);
    auto ret_idx = Embeddings::instance()->search(embed, name, k);
    return Database::instance()->getReferenceByIndices(ret_idx);
}

void Dataset::reload()
{
    for (const auto& item : list) {
        if (item.name.isEmpty() || item.path.isEmpty()) {
            LOG(warn) << "数据集配置不完整，跳过:" << item.name;
            continue;
        }

        if (!processDataset(item.name, item.path)) {
            LOG(err) << "处理数据集失败:" << item.name;
            continue;
        }
    }
    Database::instance()->clearDataset(getList());
}

bool Dataset::processDataset(const QString& name, const QString& dir_path)
{
    QDir dir(dir_path);
    if (!dir.exists()) {
        LOG(err) << "数据集目录不存在:" << dir_path;
        return false;
    }

    // 获取目录下所有文件
    auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList file_list;
    for (const auto& info : files) {
        if (!processFile(name, info.absoluteFilePath())) {
            LOG(err) << "处理文件失败:" << info.fileName();
        }
        file_list.append(info.absoluteFilePath());
    }
    Database::instance()->clearSection(name, file_list);

    return true;
}

bool Dataset::processFile(const QString& name, const QString& file_path)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG(err) << "无法打开文件:" << file_path;
        return false;
    }

    auto saved_time = Database::instance()->getSavedTime(file_path);
    if (saved_time.isValid()) {
        if (QFileInfo(file).lastModified() < saved_time) {
            // 没有修改，直接跳过
            return true;
        } else {
            // 有更新，直接删掉旧的，后续再插入新的
            Database::instance()->removeSection(file_path);
        }
    }

    QString content = file.readAll();
    content.replace(QRegularExpression(R"(\n+)"), "\n");
    auto sec_list = content.split('\n');
    for (const auto& sec : sec_list) {
        QString cleaned_text = cleanText(sec);
        if (cleaned_text.isEmpty()) {
            continue;
        }

        // 获取文本的嵌入向量
        QList<float> text_embedding = ModelAdapter::instance()->embed(cleaned_text);
        if (text_embedding.isEmpty()) {
            LOG(err) << "获取文本嵌入失败:" << cleaned_text;
            continue;
        }

        // 保存到数据库
        Reference ref_data;
        ref_data.name = name;
        ref_data.file = file_path;
        ref_data.text = cleaned_text;
        
        long save_id = Database::instance()->saveEmbedding(ref_data, text_embedding);
        if (save_id < 0) {
            LOG(err) << "保存嵌入数据失败";
            continue;
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