#include "dbstatswindow.h"
#include "ui_dbstatswindow.h"

DBStatsWindow::DBStatsWindow(DBHandler* constrDbHandler,
                             QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DBStatsWindow) {
    ui->setupUi(this);

    dbHandler = constrDbHandler;

}

DBStatsWindow::~DBStatsWindow()
{
    delete ui;
}

void DBStatsWindow::DBStatsWindowShow() {
    auto wordResults = dbHandler->FetchAllWords();
    ui->wordsTable->setRowCount(0);
    ui->wordsTable->setColumnCount(2);
    quint32 rowCount{0};
    for (const auto& [word, wordRating] : wordResults.asKeyValueRange()) {
        ui->wordsTable->insertRow(rowCount++);
        QTableWidgetItem* wordItem = new QTableWidgetItem(word);
        ui->wordsTable->setItem(rowCount - 1, 0, wordItem);
        QTableWidgetItem* wordRatingItem = new QTableWidgetItem(QString("%1").arg(wordRating));
        ui->wordsTable->setItem(rowCount - 1, 1, wordRatingItem);
    }
    ui->wordsTable->setHorizontalHeaderLabels(QStringList{"Слово", "Рейтинг"});


    this->show();
    this->move(200, 200);
    auto searchResultsWidth = ui->wordsTable->width();
    ui->wordsTable->horizontalHeader()->resizeSection(0, searchResultsWidth - 80 - 18);
    ui->wordsTable->horizontalHeader()->resizeSection(1, 80);
    ui->wordsTable->verticalHeader()->hide();
}
