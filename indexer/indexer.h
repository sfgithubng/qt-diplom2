#ifndef INDEXER_H
#define INDEXER_H

#pragma once
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include "dbhandler.h"

const QString CONFIG_IGNORE_LINE{"#["};
const QString CONFIG_ASSIGNMENT_OPERATOR{"="};
const qint32 FILE_NOT_FOUND{-1};
const quint32 INCOMPLETE_CONFIG_LIST_LENGTH{0};
const QString TEXT_INDEX_WORD_SEPARATOR{" "};
const quint32 TEXT_INDEX_MIN_WORD_LENGTH{3};
const quint32 TEXT_INDEX_MAX_WORD_LENGTH{32};

enum class ConfigSettingsVarCodes {
    uninitialized,
    fileExtensions,
    indexDirectory,
    dbHost,
    dbPort,
    dbName,
    dbUser,
    dbPass
};

class Indexer : public QObject {
    Q_OBJECT
private:
    QString configFilePath;
    QStringList configFileVars;
    QStringList configValueIgnoreChars;
    QMap<ConfigSettingsVarCodes, QString> ConfigSettingsVarNames;
    QMap<ConfigSettingsVarCodes, QString> configSettings;
    QStringList fileNameFilters;
    QVector<QString> fileNamesToIndex;
    DBHandler* dbHandler;

    bool ProcessConfigFileLine(const QString& configFileLine);
    void GetConfig();
    void CleanupStaleFilesFromDB();
    void BuildIndex();
    void SetupDB();
    QMap<QString, quint32> IndexFile(const QString& fileName);
    void GetFullFileNames();
    void UpdateIndexInDB(const QMap<QString, quint32>& fileIndex,
                         QString fileName);
public:
    explicit Indexer(QObject* parent = nullptr);
    ~Indexer();

signals:
    void HaltWorkflow(quint32 code);
    void StopWorkflow();

public slots:    
    void RunWorkflow();
};

#endif // INDEXER_H
