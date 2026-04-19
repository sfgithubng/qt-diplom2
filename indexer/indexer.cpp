#include "indexer.h"

Indexer::Indexer(QObject* parent)
    : QObject{parent} {
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::fileExtensions, "file_extensions");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::indexDirectory, "index_directory");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbHost, "db_host");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbPort, "db_port");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbName, "db_name");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbUser, "db_user");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbPass, "db_pass");

    configSettings.insert(ConfigSettingsVarCodes::fileExtensions, ".*");
    configSettings.insert(ConfigSettingsVarCodes::indexDirectory, "");
    configSettings.insert(ConfigSettingsVarCodes::dbHost, "");
    configSettings.insert(ConfigSettingsVarCodes::dbPort, "");
    configSettings.insert(ConfigSettingsVarCodes::dbName, "");
    configSettings.insert(ConfigSettingsVarCodes::dbUser, "");
    configSettings.insert(ConfigSettingsVarCodes::dbPass, "");
    configValueIgnoreChars = QStringList{
        "'",
        "\""
    };
    configFilePath = "config.ini";

    fileNamesToIndex = {};
}

Indexer::~Indexer() {
    delete dbHandler;
}

void Indexer::RunWorkflow() {
    GetConfig();
    SetupDB();
    GetFullFileNames();
    CleanupStaleFilesFromDB();
    BuildIndex();


    emit StopWorkflow();
}

void Indexer::GetConfig() {
    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не найден файл конфигурации " << configFilePath;
        qWarning() << "Его необходимо создать в соответствии с README.md";
        emit HaltWorkflow(EXIT_FAILURE);
    }
    QTextStream configFileStream(&configFile);
    QString configFileLine{};
    for (; !configFileStream.atEnd();) {
        configFileLine = configFileStream.readLine();
        if (!ProcessConfigFileLine(configFileLine)) {
            qWarning() << "Ошибка в содержимом файла конфигурации";
            qWarning() << "Строка с ошибкой: " << configFileLine;
            emit HaltWorkflow(EXIT_FAILURE);
        }
    }
}

bool Indexer::ProcessConfigFileLine(const QString& configFileLine) {
    bool returnCode;
    auto lineLength = configFileLine.length();
    switch (lineLength) {
    case 0:
        returnCode = true;
        break;
    default:
        if (CONFIG_IGNORE_LINE.contains(configFileLine.at(0))) {
            returnCode = true;
            break;
        }
        QStringList configElements{configFileLine.split(CONFIG_ASSIGNMENT_OPERATOR)};
        if (configElements.length() == INCOMPLETE_CONFIG_LIST_LENGTH) {
            returnCode = false;
            break;
        }
        QString configParam{configElements.first().trimmed()};
        auto configParamIndex{ConfigSettingsVarCodes::uninitialized};
        for (auto [settingIndex, settingName] : ConfigSettingsVarNames.asKeyValueRange()) {
            if (settingName == configParam) {
                configParamIndex = settingIndex;
                break;
            }
        }
        if (configParamIndex == ConfigSettingsVarCodes::uninitialized) {
            returnCode = false;
            break;
        }
        QString configValue{};
        for (const auto& configChar : configElements.last().trimmed()) {
            if (!configValueIgnoreChars.contains(configChar)) {
                configValue.append(configChar);
            }
        }
        configSettings[configParamIndex] = configValue;
        returnCode = true;
    }
    return returnCode;
}

void Indexer::SetupDB() {
    dbHandler = new DBHandler(configSettings.value(ConfigSettingsVarCodes::dbHost),
                                         configSettings.value(ConfigSettingsVarCodes::dbPort),
                                         configSettings.value(ConfigSettingsVarCodes::dbName),
                                         configSettings.value(ConfigSettingsVarCodes::dbUser),
                                         configSettings.value(ConfigSettingsVarCodes::dbPass));
    dbHandler->InitDB();
}

void Indexer::GetFullFileNames() {
    QString fileNameRegexString{".*"};
    if (configSettings.value(ConfigSettingsVarCodes::fileExtensions) != "") {
        QString fileExtensionsPipeSeparated = configSettings.value(ConfigSettingsVarCodes::fileExtensions).
                                              replace(QRegularExpression(",( |)"), "|");
        fileNameRegexString.append(QString(".(%1)$").arg(fileExtensionsPipeSeparated));
    }
    QRegularExpression fileNameRegex(fileNameRegexString);
    QDirIterator searchIter(configSettings.value(ConfigSettingsVarCodes::indexDirectory), QDir::Files, QDirIterator::Subdirectories);
    QString fileName{};
    while (searchIter.hasNext()) {
        fileName = searchIter.next();
        if (fileNameRegex.match(fileName.toLower()).hasMatch()) {
            fileNamesToIndex.append(fileName);
        }
    }
}

void Indexer::BuildIndex() {
    QMap<QString, quint32> fileIndex;
    for (const auto& fileName : fileNamesToIndex) {
        fileIndex = IndexFile(fileName);
        UpdateIndexInDB(fileIndex, fileName);
    }
}

QMap<QString, quint32> Indexer::IndexFile(const QString& fileName) {
    QMap<QString, quint32> fileIndex;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Ошибка индексации файла " << fileName << ", проверьте права доступа и наличие файла";
        return fileIndex;
    }
    QTextStream fileStream(&file);
    QString fileLine{};
    QStringList fileLineWords{};
    QString wordTrimmed{};
    QString punctuationMarks{"[!,./+=:;\\(\\)\"'\“*&`]+"};
    QRegularExpression lineStartPunctuationMarksRegex(QString("^%1").arg(punctuationMarks));
    QRegularExpression lineEndPunctuationMarksRegex(QString("%1$").arg(punctuationMarks));
    for (; !fileStream.atEnd();) {
        fileLine = fileStream.readLine();
        fileLineWords = fileLine.split(TEXT_INDEX_WORD_SEPARATOR);
        for (const auto& word : fileLineWords) {
            wordTrimmed = word.trimmed().toLower();
            wordTrimmed = wordTrimmed.replace(lineStartPunctuationMarksRegex, "");
            wordTrimmed = wordTrimmed.replace(lineEndPunctuationMarksRegex, "");
            if ((wordTrimmed.length() < TEXT_INDEX_MIN_WORD_LENGTH) ||
                (wordTrimmed.length() > TEXT_INDEX_MAX_WORD_LENGTH)) {
                continue;
            }
            if (fileIndex.find(wordTrimmed) == fileIndex.end()) {
                fileIndex.insert(wordTrimmed, 1);
            } else {
                ++fileIndex[wordTrimmed];
            }
        }

    }
    return fileIndex;
}

void Indexer::UpdateIndexInDB(const QMap<QString, quint32>& fileIndex,
                              QString fileName) {
    dbHandler->CleanupFileIndex(fileName);
    quint32 idFile{dbHandler->InitializeFile(fileName)};
    quint32 idWord{};
    for (const auto& [word, frequency] : fileIndex.asKeyValueRange()) {
        idWord = dbHandler->GetOrAddWord(word);
        dbHandler->InsertFileIndexEntry(idFile, idWord, frequency);
    }
}


void Indexer::CleanupStaleFilesFromDB() {
    auto fileNamesToCleanupFromDB = dbHandler->GetAllFiles();
    for (const auto& fileName : fileNamesToIndex) {
        fileNamesToCleanupFromDB.removeAll(fileName);
    }
    for (const auto& fileName : fileNamesToCleanupFromDB) {
        dbHandler->CleanupFileIndex(fileName);
        dbHandler->CleanupFile(fileName);
    }
}

