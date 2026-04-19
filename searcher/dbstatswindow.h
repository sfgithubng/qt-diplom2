#ifndef DBSTATSWINDOW_H
#define DBSTATSWINDOW_H

#pragma once
#include "dbhandler.h"
#include <QWidget>

namespace Ui {
class DBStatsWindow;
}

class DBStatsWindow : public QWidget {
    Q_OBJECT
private:
    Ui::DBStatsWindow *ui;
    DBHandler* dbHandler;
public:
    explicit DBStatsWindow(DBHandler* constrDbHandler,
                           QWidget* parent = nullptr);
    ~DBStatsWindow();
public slots:
    void DBStatsWindowShow();
};

#endif // DBSTATSWINDOW_H
