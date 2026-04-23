#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QTimer>
#include <QVector>

const qint32 WORD_NOT_FOUND{-1};

using rating = quint32;
using filePath = QString;

using DBSearchResults = QMap<rating, filePath>;
using DBWordResults = QMap<QString, rating>;

class DBHandler : public QObject {
    Q_OBJECT
private:
    QSqlDatabase* db;
    uint32_t dbConnIntervalMsec;
    QTimer* timer;
    void ConnectToDB();
signals:
    void ConnectedToDB(bool connectionStatus);

public:
    explicit DBHandler(QString dbHost,
                       QString dbPort,
                       QString dbName,
                       QString dbUser,
                       QString dbPass);
    ~DBHandler();
    DBSearchResults RunSearch(QString searchRequest);
    DBWordResults FetchAllWords();

};
#endif // DBHANDLER_H
