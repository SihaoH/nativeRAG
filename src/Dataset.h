#pragma once
#include "TypeDefine.h"
#include <QObject>

class Dataset : public QObject
{
    Q_OBJECT
public:
    Dataset(QObject* parent = nullptr);
    ~Dataset() = default;

    void init(const QString& json);
    QStringList getList() const;
    QList<Reference> search(const QString& query, const QString& name = QString(), int k = 3);

private:
    bool processDataset(const QString& name, const QString& dir_path);
    bool processFile(const QString& name, const QString& file_path);
    QString cleanText(const QString& text);

private:
    QStringList list;
};
