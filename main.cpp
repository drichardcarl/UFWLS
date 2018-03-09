#include "ufwls.h"
#include <QApplication>
#include <QDesktopWidget>

#define DB "ufwls.db"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UFWLS w;
    // check if database is present
    if (w.init(DB))
        return -1;
    w.setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            w.size(),
            a.desktop()->availableGeometry()
        )
    );
    w.showMaximized();

    return a.exec();
}
