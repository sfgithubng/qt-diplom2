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

    dbConnIntervalMsec = 100;
    timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &DBHandler::ConnectToDB);
    timer->start(dbConnIntervalMsec);
}

void DBHandler::ConnectToDB() {
    timer->stop();
    if (not db->open()) {
        QString connErrStr("Ошибка установки соединения с хостом БД. ");
        connErrStr += QString("Причина: %1").arg(db->lastError().text());
        QMessageBox connErrMsg;
        connErrMsg.setText(connErrStr);
        connErrMsg.exec();
        dbConnIntervalMsec = 5000;
        timer->start(dbConnIntervalMsec);
    } else {
        emit ConnectedToDB(true);
    }
}

DBHandler::~DBHandler() {
    db->close();
    delete db;
    delete timer;
}

DBSearchResults DBHandler::RunSearch(QString searchRequest) {
    QSqlQuery query(*db);
    query.prepare("SELECT id_document FROM documents");
    query.exec();
    QList<quint32> documentIDs{};
    for (; query.next(); ) {
        documentIDs.append(query.value(0).toUInt());
    }

    QStringList searchWords{searchRequest.split(" ")};
    auto searchRequestLen{searchWords.length()};
    QString searchQuery{"SELECT word, word_frequency FROM words, document_words "};
    searchQuery += QString("WHERE words.id_word = document_words.id_word ");
    searchQuery += QString("AND document_words.id_document = :id_document ");
    QString wordsPrepStatement{};
    for (quint32 wordNum = 0; wordNum < searchWords.length(); ++wordNum) {
        searchWords[wordNum] = searchWords.at(wordNum).toLower();
        wordsPrepStatement += QString(":word%1, ").arg(wordNum);
    }
    wordsPrepStatement.chop(QString(", ").length());
    searchQuery += QString("AND words.word IN (%1)").arg(wordsPrepStatement);
    query.prepare(searchQuery);
    for (quint32 wordNum = 0; wordNum < searchWords.length(); ++wordNum) {
        query.bindValue(QString(":word%1").arg(wordNum), searchWords.at(wordNum));
    }
    DBSearchResults searchResults;


    QSqlQuery queryFilePath(*db);
    queryFilePath.prepare("SELECT filepath FROM documents WHERE id_document = :id_document");
    for (const auto& documentID : documentIDs) {
        query.bindValue(":id_document", documentID);
        query.exec();

        QString word;
        quint32 wordIndexInSearchRequest;
        quint32 wordRating;
        quint32 resultRating{0};
        for (; query.next(); ) {
            word = query.value(0).toString();
            wordIndexInSearchRequest = searchWords.indexOf(word);
            wordRating = query.value(1).toUInt();
            resultRating += wordRating * (searchRequestLen - wordIndexInSearchRequest);
        }
        if (resultRating == 0) {
            continue;
        }
        queryFilePath.bindValue(":id_document", documentID);
        queryFilePath.exec();
        queryFilePath.first();
        QString filePath{queryFilePath.value(0).toString()};
        searchResults.insert(resultRating, filePath);

    }
    return searchResults;
}

DBWordResults DBHandler::FetchAllWords() {
    QSqlQuery query(*db);
    QString wordsQuery{"SELECT word, SUM(word_frequency) "};
    wordsQuery += QString("FROM words, document_words ");
    wordsQuery += QString("WHERE words.id_word = document_words.id_word ");
    wordsQuery += QString("GROUP BY word ORDER BY word");
    query.prepare(wordsQuery);
    query.exec();
    DBWordResults wordResults{};
    for (; query.next(); ) {
        wordResults.insert(query.value(0).toString(), query.value(1).toUInt());
    }
    return wordResults;
}
