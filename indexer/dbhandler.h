#ifndef DBHANDLER_H
#define DBHANDLER_H

#pragma once
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QThread>

class DBHandler {
private:
    QSqlDatabase* db;
    quint32 dbConnIntervalMsec;
public:
    DBHandler(QString dbHost,
              QString dbPort,
              QString dbName,
              QString dbUser,
              QString dbPass);
    ~DBHandler();
    void InitDB();
    void CleanupFileIndex(QString fileName);
    quint32 InitializeFile(QString fileName);
    void CleanupFile(QString fileName);
    quint32 GetOrAddWord(QString word);
    void InsertFileIndexEntry(quint32 idFile,
                              quint32 idWord,
                              quint32 frequency);
    QList<QString> GetAllFiles();
};

#endif // DBHANDLER_H
