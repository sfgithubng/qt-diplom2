#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    appInitialized = false;
    ui->setupUi(this);
    ui->searchQueryEdit->setText(DEFAULT_SEARCH_CONTENT);
    ui->searchQueryEdit->setStyleSheet("color:grey");
    ui->searchQueryRun->setText("Поиск");
    ui->searchQueryRun->setEnabled(false);
    ui->searchResults->setVisible(false);
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbHost, "db_host");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbPort, "db_port");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbName, "db_name");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbUser, "db_user");
    ConfigSettingsVarNames.insert(ConfigSettingsVarCodes::dbPass, "db_pass");

    configSettings.insert(ConfigSettingsVarCodes::dbHost, "");
    configSettings.insert(ConfigSettingsVarCodes::dbPort, "");
    configSettings.insert(ConfigSettingsVarCodes::dbName, "");
    configSettings.insert(ConfigSettingsVarCodes::dbUser, "");
    configSettings.insert(ConfigSettingsVarCodes::dbPass, "");

    configValueIgnoreChars = QStringList{
        "'",
        "\""
    };

    mainWindowMenuBar = new QMenuBar;
    wordStatsAction = new QAction;
    wordStatsAction->setEnabled(false);
    wordStatsAction->setText("Просмотр слов и их частоты в БД");
    mainWindowMenuBar->addAction(wordStatsAction);
    this->setMenuBar(mainWindowMenuBar);

    mainWindowStatusBar = new QStatusBar();
    this->setStatusBar(mainWindowStatusBar);
    mainWindowStatusBar->addPermanentWidget(ui->dbConnStatusLabel);

    QObject::connect(this, &MainWindow::HaltWorkflow, this, &QCoreApplication::exit, Qt::QueuedConnection);
    configFilePath = "config.ini";
    GetConfig();
    DBConnectUIUpdate(false);


    dbHandler = new DBHandler(configSettings.value(ConfigSettingsVarCodes::dbHost),
                              configSettings.value(ConfigSettingsVarCodes::dbPort),
                              configSettings.value(ConfigSettingsVarCodes::dbName),
                              configSettings.value(ConfigSettingsVarCodes::dbUser),
                              configSettings.value(ConfigSettingsVarCodes::dbPass));
    QObject::connect(dbHandler, &DBHandler::ConnectedToDB, this, &MainWindow::DBConnectUIUpdate);
    QObject::connect(ui->searchQueryEdit, &QLineEdit::returnPressed, this, &MainWindow::on_searchQueryRun_clicked);

    dbStatsWindow = new DBStatsWindow(dbHandler);
    dbStatsWindow->setWindowTitle("Просмотр слов и их частоты в БД");
    QObject::connect(wordStatsAction, &QAction::triggered, dbStatsWindow, &DBStatsWindow::DBStatsWindowShow);
    this->move(200, 200);
}

MainWindow::~MainWindow() {
    delete ui;
    delete mainWindowMenuBar;
    delete wordStatsAction;
    delete mainWindowStatusBar;
    delete dbStatsWindow;
}

bool MainWindow::ProcessConfigFileLine(const QString& configFileLine) {
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

void MainWindow::GetConfig() {
    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString configFileErr(QString("Не найден файл конфигурации %1").arg(configFilePath));
        configFileErr += QString("Его необходимо создать в соответствии с README.md");
        QMessageBox configFileErrMsg;
        configFileErrMsg.setText(configFileErr);
        configFileErrMsg.exec();
        emit HaltWorkflow(EXIT_FAILURE);
    }
    QTextStream configFileStream(&configFile);
    QString configFileLine{};
    for (; !configFileStream.atEnd();) {
        configFileLine = configFileStream.readLine();
        if (!ProcessConfigFileLine(configFileLine)) {
            QString configFileErr("Ошибка в содержимом файла конфигурации");
            configFileErr += QString("Строка с ошибкой: %1").arg(configFileLine);
            QMessageBox configFileErrMsg;
            configFileErrMsg.setText(configFileErr);
            configFileErrMsg.exec();
            emit HaltWorkflow(EXIT_FAILURE);
        }
    }
}

void MainWindow::DBConnectUIUpdate(bool connectionStatus) {
    QMap<bool, QString> dbConnStatusToLabel{};
    dbConnStatusToLabel.insert(true, "Подключено к БД");
    dbConnStatusToLabel.insert(false, "Отключено от БД");

    QMap<bool, QString> dbConnStatusToLabelStyle{};
    dbConnStatusToLabelStyle.insert(true, "color:green");
    dbConnStatusToLabelStyle.insert(false, "color:red");


    ui->dbConnStatusLabel->setText(dbConnStatusToLabel.value(connectionStatus));
    ui->dbConnStatusLabel->setStyleSheet(dbConnStatusToLabelStyle.value(connectionStatus));
    ui->searchQueryEdit->setEnabled(connectionStatus);
    wordStatsAction->setEnabled(connectionStatus);
    appInitialized = connectionStatus;

}

void MainWindow::on_searchQueryEdit_cursorPositionChanged() {
    OnSearchQueryUIChange();
}


void MainWindow::on_searchQueryEdit_textChanged() {
    OnSearchQueryUIChange();
}

void MainWindow::OnSearchQueryUIChange() {
    if (!appInitialized) {
        return;
    }

    if (ui->searchQueryEdit->styleSheet() == "color:grey") {
        ui->searchQueryEdit->setStyleSheet("color:black");
        auto searchFieldContent{ui->searchQueryEdit->text()};
        if (searchFieldContent == DEFAULT_SEARCH_CONTENT) {
            ui->searchQueryEdit->setText("");
        } else {
            QStringList searchFieldItems{searchFieldContent.split(DEFAULT_SEARCH_CONTENT)};
            ui->searchQueryEdit->setText(searchFieldItems.last().trimmed());
        }
    }
    if (!ui->searchQueryRun->isEnabled() && ui->searchQueryEdit->text() != "") {
        ui->searchQueryRun->setEnabled(true);
        return;
    }
    if (ui->searchQueryRun->isEnabled() && ui->searchQueryEdit->text() == "") {
        ui->searchQueryRun->setEnabled(false);
        return;
    }
}


void MainWindow::on_searchQueryRun_clicked() {
    ui->searchResults->setVisible(true);
    auto dbSearchResults = dbHandler->RunSearch(ui->searchQueryEdit->text());
    if (dbSearchResults.isEmpty()) {
        return;
    }
    ui->searchResults->setRowCount(0);
    ui->searchResults->setColumnCount(1);
    quint32 rowCount{0};
    auto dbSearchResultsIter = --(dbSearchResults.keyValueEnd());
    quint32 maxRating{(*dbSearchResultsIter).first};
    for (; dbSearchResultsIter != dbSearchResults.keyValueBegin(); --dbSearchResultsIter) {
        AddEntryToSearchResults(*dbSearchResultsIter, rowCount, maxRating);
    }
    AddEntryToSearchResults(*dbSearchResults.keyValueBegin(), rowCount, maxRating);
    auto searchResultsWidth = ui->searchResults->width();
    ui->searchResults->setHorizontalHeaderLabels(QStringList{"Результаты поиска"});
    ui->searchResults->horizontalHeader()->resizeSection(0, searchResultsWidth - 10);
    ui->searchResults->verticalHeader()->hide();
}

void MainWindow::AddEntryToSearchResults(QPair<rating, filePath> seachEntryVals,
                                         quint32& rowCount,
                                         const quint32& maxRating) {
    quint32 relativeRating{seachEntryVals.first*100/maxRating};
    QString searchResultFileText = QString("Файл: %1").arg(seachEntryVals.second);
    QString searchResultRelevanceText = QString("Относительное соответствие: %1/100").arg(relativeRating);
    ui->searchResults->insertRow(rowCount++);
    QWidget* cellWidget = new QWidget;
    QVBoxLayout* cellVBox = new QVBoxLayout(cellWidget);
    QLabel* cellLabel;
    cellLabel = new QLabel(searchResultFileText);
    cellVBox->addWidget(cellLabel);
    cellLabel = new QLabel(searchResultRelevanceText);
    cellVBox->addWidget(cellLabel);
    ui->searchResults->setCellWidget(rowCount - 1, 0, cellWidget);
    ui->searchResults->resizeRowToContents(rowCount - 1);
}

