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
#include <QSerialPort>

#include "alert.hpp"
#include "chart.h"
#include "winaddcontact.h"
#include "dbmanager.hpp"
#include "settingsdialog.h"
#include "about.h"

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
    struct SMS{
        QString contactName;
        QString contactNum;
        QString msg;
    };

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
    void updateLastRowClicked();
    void initSerialDeviceConn();
    void xSerialDeviceConn();
    void scheduleSMS();

private slots:
    void on_configBtn_clicked();

    void on_notifyBtn_clicked();

private:
    void _load(int loadingMode = -1);
    void _setup();
    void _setupTrayIcon();
    void _enWidgets(int val);

    Ui::UFWLS *ui;
    QTimer mon;
    QTimer smsScheduler;
    Chart* chart;
    WinAddContact* AddContactWindow;
    DbManager* dbmngr;
    QSystemTrayIcon* trayIcon;
    QTableWidgetItem* lastItem;
    about* winAbout;
    int lastRow;

    SettingsDialog* settings;
    QSerialPort* serial;
    // SMS_BUFFER = [SMS,...]
    // SMS = [NAME, CONTACT #, MESSAGE]
    QList<SMS> smsBuffer;
    int smsIndex;


};

#endif // UFWLS_H
