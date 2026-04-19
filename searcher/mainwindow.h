#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include "dbhandler.h"
#include "dbstatswindow.h"
#include <QFile>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QPair>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

const QString CONFIG_IGNORE_LINE{"#["};
const QString CONFIG_ASSIGNMENT_OPERATOR{"="};
const QString DEFAULT_SEARCH_CONTENT{"Введите запрос"};
const qint32 FILE_NOT_FOUND{-1};
const quint32 INCOMPLETE_CONFIG_LIST_LENGTH{0};

enum class ConfigSettingsVarCodes {
    uninitialized,
    dbHost,
    dbPort,
    dbName,
    dbUser,
    dbPass
};

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QString configFilePath;
    QStringList configFileVars;
    QStringList configValueIgnoreChars;
    QMap<ConfigSettingsVarCodes, QString> ConfigSettingsVarNames;
    QMap<ConfigSettingsVarCodes, QString> configSettings;
    Ui::MainWindow *ui;
    DBHandler* dbHandler;
    QAction* wordStatsAction;
    QMenuBar* mainWindowMenuBar;
    QStatusBar* mainWindowStatusBar;
    DBStatsWindow* dbStatsWindow;
    bool appInitialized;
    bool ProcessConfigFileLine(const QString& configFileLine);
    void GetConfig();
    void OnSearchQueryUIChange();
    void AddEntryToSearchResults(QPair<rating, filePath> seachEntryVals,
                                 quint32& rowCount,
                                 const quint32& maxRating);

private slots:
    void DBConnectUIUpdate(bool connectionStatus);
    void on_searchQueryEdit_cursorPositionChanged();

    void on_searchQueryEdit_textChanged();

    void on_searchQueryRun_clicked();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void HaltWorkflow(quint32 code);
};
#endif // MAINWINDOW_H
