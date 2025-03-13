#pragma once
#include "TypeDefine.h"
#include <QSqlDatabase>
#include <QDateTime>
#include <QMap>

class Database
{
public:
    static Database* instance();

    QDateTime getSavedTime(const QString& file);
    void removeDataset(const QString& name);
    void removeSection(const QString& file);
    void clearDataset(const QStringList& cur_list);
    void clearSection(const QString& name, const QStringList& cur_list);
    long saveEmbedding(const Reference& section, const QList<float>& embedding);
    QMap<long, QList<float>> loadEmbeddings(const QString& name);
    QList<Reference> getReferenceByIndices(const QList<long>& indices);

private:
    Database();
    ~Database();
    QSqlDatabase& getDatabase();
    bool initializeDatabase();

private:
    const QString DB_FILENAME = "dataset.db";
    QSqlDatabase db;
};
