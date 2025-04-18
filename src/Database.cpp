#include "Database.h"
#include "Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>

Database::Database()
{
    // 检查数据库文件是否存在
    bool dbExists = QFile::exists(DB_FILENAME);
    
    // 创建数据库连接
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(DB_FILENAME);
    
    // 打开数据库连接
    if (!db.open()) {
        LOG(err) << "无法打开数据库:" << db.lastError().text();
        return;
    }
    
    // 如果数据库文件不存在，初始化数据库结构
    if (!dbExists) {
        if (!initializeDatabase()) {
            LOG(err) << "初始化数据库失败";
            db.close();
            return;
        }
    }
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

Database* Database::instance()
{
    static Database db;
    return &db;
}

QSqlDatabase& Database::getDatabase()
{
    return db;
}

bool Database::initializeDatabase()
{
    QSqlQuery query(db);
    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS Embeddings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "file TEXT NOT NULL,"
        "text TEXT NOT NULL,"
        "vector BLOB NOT NULL,"
        "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")"
    );

    if (!success) {
        LOG(err) << "创建Embeddings表失败:" << query.lastError().text();
    }
    return success;
}

QDateTime Database::getSavedTime(const QString& file)
{
    QSqlQuery query(db);
    query.prepare("SELECT MAX(time) FROM Embeddings WHERE file = ?");
    query.addBindValue(file);
    
    if (!query.exec()) {
        LOG(err) << "查询文件保存时间失败:" << query.lastError().text();
        return QDateTime();
    }
    
    if (query.next()) {
        return query.value(0).toDateTime();
    }
    
    return QDateTime();
}

void Database::removeDataset(const QString& name)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM Embeddings WHERE name = ?");
    query.addBindValue(name);
    
    if (!query.exec()) {
        LOG(err) << "删除数据集记录失败:" << query.lastError().text();
    }
}

void Database::removeSection(const QString& file)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM Embeddings WHERE file = ?");
    query.addBindValue(file);
    
    if (!query.exec()) {
        LOG(err) << "删除文件记录失败:" << query.lastError().text();
    }
}

void Database::clearDataset(const QStringList& cur_list)
{
    // 获取数据库中所有的dataset名称
    QSqlQuery query(db);
    if (!query.exec("SELECT DISTINCT name FROM Embeddings")) {
        LOG(err) << "查询数据集列表失败:" << query.lastError().text();
        return;
    }

    // 检查哪些dataset不在当前列表中
    while (query.next()) {
        QString name = query.value(0).toString();
        if (!cur_list.contains(name)) {
            // 删除不存在的dataset
            removeDataset(name);
            LOG(info) << "删除不存在的数据集:" << name;
        }
    }
}

void Database::clearSection(const QString& name, const QStringList& cur_list)
{
    // 获取该dataset下所有文件路径
    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT file FROM Embeddings WHERE name = ?");
    query.addBindValue(name);
    
    if (!query.exec()) {
        LOG(err) << "查询文件列表失败:" << query.lastError().text();
        return;
    }

    // 检查哪些文件不在当前列表中
    while (query.next()) {
        QString file = query.value(0).toString();
        if (!cur_list.contains(file)) {
            // 删除不存在的文件记录
            removeSection(file);
            LOG(info) << "删除不存在的文件记录:" << file;
        }
    }
}

long Database::saveEmbedding(const Reference& ref, const QList<float>& embedding)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO Embeddings (name, file, text, vector) VALUES (?, ?, ?, ?)");
    QByteArray vector_data(reinterpret_cast<const char*>(embedding.data()), embedding.size() * sizeof(float));
    query.addBindValue(ref.name);
    query.addBindValue(ref.file);
    query.addBindValue(ref.text);
    query.addBindValue(vector_data);

    if (!query.exec()) {
        LOG(err) << "保存向量到数据库失败:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toLongLong();
}

QMap<long, QList<float>> Database::loadEmbeddings(const QString& name)
{
    QSqlQuery query(db);
    // 根据name参数构建查询语句
    QString sql_query = "SELECT id, vector FROM Embeddings";
    if (!name.isEmpty()) {
        sql_query += " WHERE name = :name";
        query.prepare(sql_query);
        query.bindValue(":name", name);
    } else {
        query.prepare(sql_query);
    }

    QMap<long, QList<float>> embed_map;
    if (!query.exec()) {
        LOG(err) << "从数据库加载向量失败:" << query.lastError().text();
        return embed_map;
    }

    while (query.next()) {
        long id = query.value(0).toLongLong();
        QByteArray vector_data = query.value(1).toByteArray();
        const float* vector = reinterpret_cast<const float*>(vector_data.constData());
        const size_t size = vector_data.size() / sizeof(float);
        std::vector<float> ret(vector, vector + size);
        embed_map.insert(id, QList<float>(ret.begin(), ret.end()));
    }

    return embed_map;
}

QList<Reference> Database::getReferenceByIndices(const QList<long>& indices)
{
    QList<Reference> results;
    for (long id : indices) {
        if (id < 0) continue;

        QSqlQuery query(db);
        query.prepare("SELECT name, file, text FROM Embeddings WHERE id = ?");
        query.addBindValue(QVariant::fromValue<long>(id));
        
        if (query.exec() && query.next()) {
            Reference ref;
            ref.name = query.value(0).toString();
            ref.file = query.value(1).toString();
            ref.text = query.value(2).toString();
            results.append(ref);
        }
    }

    return results;
}
