#ifndef DBSTATSWINDOW_H
#define DBSTATSWINDOW_H

#include "dbhandler.h"
#include <QWidget>

const quint32 DB_WINDOW_OFFSET{200};
const quint32 WORDS_TABLE_WORD_COLUMN_WIDTH{80};
const quint32 WORDS_TABLE_SCROLL_AREA_WIDTH{18};
const quint32 EMPTY_WORDS_TABLE{0};
const quint32 WORD_COLUMN{0};
const quint32 RATING_COLUMN{1};

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
