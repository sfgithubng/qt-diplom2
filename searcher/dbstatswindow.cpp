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
    delete dbHandler;
}

void DBStatsWindow::DBStatsWindowShow() {
    auto wordResults = dbHandler->FetchAllWords();
    QStringList wordsTableHeaders{"Слово", "Рейтинг"};
    ui->wordsTable->setRowCount(EMPTY_WORDS_TABLE);
    ui->wordsTable->setColumnCount(wordsTableHeaders.size());
    quint32 rowCount{0};
    for (const auto& [word, wordRating] : wordResults.asKeyValueRange()) {
        ui->wordsTable->insertRow(rowCount++);
        QTableWidgetItem* wordItem = new QTableWidgetItem(word);
        ui->wordsTable->setItem(rowCount - 1, WORD_COLUMN, wordItem);
        QTableWidgetItem* wordRatingItem = new QTableWidgetItem(QString("%1").arg(wordRating));
        ui->wordsTable->setItem(rowCount - 1, RATING_COLUMN, wordRatingItem);
        //delete wordItem;
        //delete wordRatingItem;
    }
    ui->wordsTable->setHorizontalHeaderLabels(wordsTableHeaders);


    this->show();
    this->move(DB_WINDOW_OFFSET, DB_WINDOW_OFFSET);
    auto searchResultsWidth = ui->wordsTable->width();
    auto ratingColumnWidth = searchResultsWidth - WORDS_TABLE_WORD_COLUMN_WIDTH - WORDS_TABLE_SCROLL_AREA_WIDTH;
    ui->wordsTable->horizontalHeader()->resizeSection(0, ratingColumnWidth);
    ui->wordsTable->horizontalHeader()->resizeSection(1, WORDS_TABLE_WORD_COLUMN_WIDTH);
    ui->wordsTable->verticalHeader()->hide();
}
