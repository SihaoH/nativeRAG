#pragma once
#include "TypeDefine.h"
#include <QSqlDatabase>
#include <QMap>

class Database
{
public:
    static Database* instance();

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
