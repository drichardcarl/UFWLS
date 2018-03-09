#include "ufwls.h"
#include "ui_ufwls.h"

UFWLS::UFWLS(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UFWLS),
    AddContactWindow(new WinAddContact),
    trayIcon(nullptr)
{
    ui->setupUi(this);
}

UFWLS::~UFWLS()
{
    delete ui;
}

int UFWLS::init(const QString& db){
    dbmngr = new DbManager;
    if (dbmngr->open(db))
        return -1;

    _setup();
    _load();
    return 0;
}

void UFWLS::_setup(){
    QObject::connect(&mon, &QTimer::timeout, this, &UFWLS::handleTimeout);
    QObject::connect(ui->txtEmergencyTxtMsg, &QPlainTextEdit::textChanged, this, &UFWLS::updateCharCount);
    QObject::connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &UFWLS::editContact);
//    QObject::connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &UFWLS::checkNumberFormat);
    mon.setInterval(1000);

    QFont fontTitle("Calibri",15,QFont::Bold);
    chart = new Chart;
    chart->setTitle("<span style=\"color:#0D47A1;\">Ultrasonic Flood Water Level Sensor (UFWLS)</span>");
    chart->setTitleFont(fontTitle);
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::AllAnimations);


    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->gridLayout->addWidget(chartView, 1, 0);
//    QSplineSeries *series = new QSplineSeries;

//    QDateTime xValue;
//    xValue.setDate(QDate(2012, 1 , 18));
//    xValue.setTime(QTime(9, 34));
//    qreal yValue = 12;
//    series->append(xValue.toMSecsSinceEpoch(), yValue);

//    xValue.setDate(QDate(2012, 1 , 18));
//    xValue.setTime(QTime(9, 36));
//    yValue = 22;
//    series->append(xValue.toMSecsSinceEpoch(), yValue);

//    QChartView *chartView = new QChartView;
//    chartView->chart()->addSeries(series);
//    chartView->chart()->createDefaultAxes();

//    // ...
//    QDateTimeAxis *axisX = new QDateTimeAxis;
//    axisX->setFormat("HH:mm");
//    chartView->chart()->setAxisX(axisX, series);


//    chartView->chart()->scroll(0,0);

//    ui->gridLayout->addWidget(chartView, 1, 0);
    mon.start();
    ui->tableWidget->installEventFilter(this);
    AddContactWindow->setDbManager(dbmngr);
    _setupTrayIcon();
    chart->setTrayIcon(this->trayIcon);
}

void UFWLS::_load(){
    QSqlQuery query;
    query.exec("SELECT contactName, contactNumber FROM contacts");
    int i = 0;
    while(query.next()){
        ui->tableWidget->setRowCount(i+1);
        for (int j=0; j<2; ++j){
            QTableWidgetItem* item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setText(query.value(j).toString());
            qDebug() << query.value(j).toString();
            ui->tableWidget->setItem(i, j, item);
        }
        ++i;
    }
    if (i == 0) ui->tableWidget->setRowCount(0);
}

void UFWLS::handleTimeout()
{
    ui->lblAlertLevel->setText("alert level [<b>" + chart->alertLevel + "</b>]");
    ui->lblSensorReading->setText("sensor reading [<b>" +
                                  QString::number(chart->distance,'f',2) + " m</b> "
                                  "@ <span style=\"color:#3949AB\">" + chart->strDateTime + "</span>]");
}

void UFWLS::updateCharCount(){
    QString currentText = ui->txtEmergencyTxtMsg->document()->toPlainText();
    int currentCharCount = currentText.size();
    int remainingCharCount = 160-currentCharCount;
    remainingCharCount = (remainingCharCount < 0) ? 0 : remainingCharCount;

    ui->lblCharCount->setText("<b>" + QString::number(remainingCharCount) + "</b> characters left.");

    if (currentCharCount > 160){
        QString text160 = currentText.left(160);
        ui->txtEmergencyTxtMsg->setPlainText(text160);

        QTextCursor cursor = ui->txtEmergencyTxtMsg->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->txtEmergencyTxtMsg->setTextCursor(cursor);
    }



}

void UFWLS::editContact(QTableWidgetItem* item){
    QString cname = ui->tableWidget->item(item->row(), 0)->text();
    QString cnum = ui->tableWidget->item(item->row(), 1)->text();
    AddContactWindow->configForEdit(cname, cnum);

    if (AddContactWindow->exec() == QDialog::Accepted)
        _load();
}

void UFWLS::checkNumberFormat(QTableWidgetItem* item){
    qDebug() << "activated!";
    // column 1 is contact number
    if (item->column() == 1){
        QRegExp re("\\d*");
        qDebug() << "loop";
        if (!re.exactMatch(item->text())){
            qDebug() << ui->tableWidget->currentIndex();
            return;
        }
    }
//    if (col == 1){
//        QRegExp re("\\d*");
//        QTableWidgetItem* item = ui->tableWidget->item(row,col);
//        while(1){
//            qDebug() << "loop";
//            if (!re.exactMatch(item->text())){
//                alert(0,"","");
//            }
//            else break;
//        }
//    }
}

void UFWLS::keyPressEvent(QKeyEvent *event){
    qDebug() << "key pressed!" << event->key();
    if (event->key() == Qt::Key_Delete){
        if (ui->tableWidget->hasFocus()){
            QList<QTableWidgetItem*> selected = ui->tableWidget->selectedItems();
            qDebug() << selected;
            if (selected.size()){
                QString name, contactNo;
                name = selected.at(0)->text();
                contactNo = selected.at(1)->text();
                qint32 res = alert(3,
                                "Critical Operation!",
                                "Are you sure you want to remove this emergency contact?<br>"
                                "Name : <b>" + name + "</b><br>"
                                "Contact # : <b>" + contactNo + "</b>");

                if (res == QMessageBox::Yes){
                    dbmngr->deleteContact(contactNo);
                    _load();
                }
            }
        }
    }

    if (event->key() == Qt::Key_F2){
        if (ui->tableWidget->hasFocus()){
//            int rowCount = ui->tableWidget->rowCount();
//            QTableWidgetItem* lastContactName = ui->tableWidget->item(rowCount-1, 0);
//            QTableWidgetItem* lastContactNum = ui->tableWidget->item(rowCount-1, 1);
//            if (lastContactName->text() == "" || lastContactNum->text() == ""){
//                alert(1,
//                      "Invalid Operation!",
//                      "Please fill out the last row first before adding a new row.");
//            }
//            else {
//                ui->tableWidget->insertRow(rowCount);
//                ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(""));
//                ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(""));
//            }
        }
        AddContactWindow->configForAdd();
        int res = AddContactWindow->exec();
        if (res == QDialog::Accepted){
//            QList<QString> contactInfo = AddContactWindow->getContactInfo();
////            int rowCount = ui->tableWidget->rowCount();

////            ui->tableWidget->insertRow(rowCount);
////            ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(contactInfo.at(0)));
////            ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(contactInfo.at(1)));
//            dbmngr->addContact(contactInfo.at(0), contactInfo.at(1));
            _load();
        }
    }
}

void UFWLS::_setupTrayIcon(){
    trayIcon = new QSystemTrayIcon(QIcon(":res/ico/ufwls.ico"), this);

    QObject::connect(trayIcon, &QSystemTrayIcon::activated, this, &UFWLS::onShowHideTrayDbClick);

    QAction *quitAction = new QAction( "Exit", trayIcon );
    QObject::connect(quitAction, &QAction::triggered, this, &UFWLS::onExit);

    QAction *hideAction = new QAction( "Show/Hide", trayIcon );
    QObject::connect(hideAction, &QAction::triggered, this, &UFWLS::onShowHide);

    QMenu *trayIconMenu = new QMenu;
    trayIconMenu->addAction( hideAction );
    trayIconMenu->addAction( quitAction );

    trayIcon->setContextMenu( trayIconMenu );

    trayIcon->show();
}

void UFWLS::onShowHideTrayDbClick(QSystemTrayIcon::ActivationReason reason){
    if( reason ) {
        if( reason != QSystemTrayIcon::DoubleClick )
        return;
    }

    if( isVisible() ) {
        hide();
    }
    else  {
        show();
        raise();
        setFocus();
    }
}

void UFWLS::onExit(bool checked){
    exit(EXIT_SUCCESS);
}


void UFWLS::onShowHide(bool checked){
    if( isVisible() ) {
        hide();
    }
    else  {
        show();
        raise();
        setFocus();
    }
}

void UFWLS::closeEvent(QCloseEvent* event){
    if( isVisible() ) {
        hide();
    }
    else  {
        show();
        raise();
        setFocus();
    }

    event->ignore();
}

