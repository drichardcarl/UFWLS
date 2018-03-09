#ifndef UFWLS_H
#define UFWLS_H

#include <QWidget>
#include <QGridLayout>
#include <QChart>
#include <QChartView>
#include <QSplineSeries>
#include <QValueAxis>
#include <QList>
//#include <QRandomGenerator>
#include <QDebug>
#include <QTextDocument>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTableWidgetItem>
#include <QRegExp>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QCloseEvent>

#include "alert.hpp"
#include "chart.h"
#include "winaddcontact.h"
#include "dbmanager.hpp"

typedef QPair<QPointF, QString> Data;
typedef QList<Data> DataList;
typedef QList<DataList> DataTable;

QT_CHARTS_USE_NAMESPACE
namespace Ui {
class UFWLS;
}

class UFWLS : public QWidget
{
    Q_OBJECT

public:
    explicit UFWLS(QWidget *parent = 0);
    ~UFWLS();

    int init(const QString& db);

protected:
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent* event);
//    bool event(QEvent *event);

;
public slots:
    void handleTimeout();
    void updateCharCount();
    void checkNumberFormat(QTableWidgetItem* item);
    void editContact(QTableWidgetItem* item);
    void onShowHideTrayDbClick(QSystemTrayIcon::ActivationReason reason);
    void onShowHide(bool checked);
    void onExit(bool checked);

private:
    void _load();
    void _setup();
    void _setupTrayIcon();

    Ui::UFWLS *ui;
    QTimer mon;
    Chart* chart;
    WinAddContact* AddContactWindow;
    DbManager* dbmngr;
    QSystemTrayIcon* trayIcon;

};

#endif // UFWLS_H
