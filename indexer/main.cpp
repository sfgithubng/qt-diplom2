#include <QCoreApplication>
#include <QTimer>

#include "indexer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Indexer* indexer = new Indexer();

    quint32 msecTimeout = 0;

    QObject::connect(indexer, &Indexer::StopWorkflow, &a, &QCoreApplication::quit, Qt::QueuedConnection);
    QObject::connect(indexer, &Indexer::HaltWorkflow, &a, &QCoreApplication::exit, Qt::QueuedConnection);
    QTimer::singleShot(msecTimeout, indexer, &Indexer::RunWorkflow);

    return a.exec();
}
