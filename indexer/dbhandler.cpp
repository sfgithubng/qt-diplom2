#include "dbhandler.h"

DBHandler::DBHandler(QString dbHost,
              QString dbPort,
              QString dbName,
              QString dbUser,
              QString dbPass) {
    db = new QSqlDatabase();
    *db = QSqlDatabase::addDatabase("QPSQL", dbName);
    db->setHostName(dbHost);
    db->setPort(dbPort.toInt());
    db->setDatabaseName(dbName);
    db->setUserName(dbUser);
    db->setPassword(dbPass);

    dbConnIntervalMsec = 5000;
    for (; !db->open(); ) {
        qWarning() << "Ошибка установки соединения с хостом БД, причина: " << db->lastError().text();
        QThread::msleep(dbConnIntervalMsec);
    }
}

DBHandler::~DBHandler() {
    db->close();
    delete db;
}

void DBHandler::InitDB() {
    QSqlQuery query(*db);
    QString queryString{};

    queryString = "CREATE TABLE IF NOT EXISTS documents ("
                  "id_document serial CHECK (id_document > 0),"
                  "filepath varchar(4096) NOT NULL,"
                  "PRIMARY KEY (id_document)"
                  ")";
    query.exec(queryString);

    queryString = "CREATE TABLE IF NOT EXISTS words ("
                  "id_word serial CHECK (id_word > 0),"
                  "word varchar(32) NOT NULL,"
                  "PRIMARY KEY (id_word)"
                  ")";
    query.exec(queryString);

    queryString = "CREATE TABLE IF NOT EXISTS document_words ("
                  "id_document int,"
                  "id_word int,"
                  "word_frequency int CHECK (word_frequency > 0),"
                  "PRIMARY KEY (id_document, id_word),"
                  "FOREIGN KEY (id_document) REFERENCES documents(id_document) ON DELETE CASCADE,"
                  "FOREIGN KEY (id_word) REFERENCES words(id_word) ON DELETE CASCADE"
                  ")";
    query.exec(queryString);
}

void DBHandler::CleanupFileIndex(QString fileName) {
    QSqlQuery query(*db);
    query.prepare("SELECT COUNT(id_document) FROM documents WHERE filepath = :filepath");
    query.bindValue(":filepath", fileName);
    query.exec();
    query.first();
    quint32 documentCount{query.value(0).toUInt()};
    if (documentCount > 0) {
        query.prepare("DELETE FROM document_words WHERE id_document IN "
                  "(SELECT id_document from documents WHERE filepath = :filepath)");
        query.bindValue(":filepath", fileName);
        query.exec();
    }
}

quint32 DBHandler::InitializeFile(QString fileName) {
    QSqlQuery query(*db);
    query.prepare("SELECT COUNT(id_document) FROM documents WHERE filepath = :filepath");
    query.bindValue(":filepath", fileName);
    query.exec();
    query.first();
    quint32 documentCount{query.value(0).toUInt()};
    if (documentCount == 0) {
        query.prepare("INSERT INTO documents (filepath) VALUES (:filepath)");
        query.bindValue(":filepath", fileName);
        query.exec();
    }

    query.prepare("SELECT id_document FROM documents WHERE filepath = :filepath");
    query.bindValue(":filepath", fileName);
    query.exec();
    query.first();
    quint32 idDocument{query.value(0).toUInt()};

    return idDocument;
}

void DBHandler::CleanupFile(QString fileName) {
    QSqlQuery query(*db);
    query.prepare("DELETE FROM documents WHERE filepath = :filepath");
    query.bindValue(":filepath", fileName);
    query.exec();
}

quint32 DBHandler::GetOrAddWord(QString word) {
    QSqlQuery query(*db);
    query.prepare("SELECT COUNT(id_word) FROM words WHERE word = :word");
    query.bindValue(":word", word);
    query.exec();
    query.first();
    quint32 wordCount{query.value(0).toUInt()};
    if (wordCount == 0) {
        query.prepare("INSERT INTO words (word) VALUES (:word)");
        query.bindValue(":word", word);
        query.exec();
    }
    query.prepare("SELECT id_word FROM words WHERE word = :word");
    query.bindValue(":word", word);
    query.exec();
    query.first();
    quint32 idWord{query.value(0).toUInt()};

    return idWord;
}

void DBHandler::InsertFileIndexEntry(quint32 idFile,
                                     quint32 idWord,
                                     quint32 frequency) {
    QSqlQuery query(*db);
    query.prepare("INSERT INTO document_words (id_document, id_word, word_frequency) "
                  "VALUES (:id_document, :id_word, :word_frequency)");
    query.bindValue(":id_document", idFile);
    query.bindValue(":id_word", idWord);
    query.bindValue(":word_frequency", frequency);
    query.exec();
}

QList<QString> DBHandler::GetAllFiles() {
    QSqlQuery query(*db);
    query.prepare("SELECT filepath FROM documents");
    query.exec();
    QList<QString> filesNamesInDB{};
    for (; query.next(); ) {
        filesNamesInDB.append(query.value(0).toString());
    }

    return filesNamesInDB;
}
