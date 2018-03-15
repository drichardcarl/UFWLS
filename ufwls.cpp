#include "ufwls.h"
#include "ui_ufwls.h"

UFWLS::UFWLS(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UFWLS),
    AddContactWindow(new WinAddContact),
    trayIcon(nullptr),
    lastRow(0),
    lastItem(nullptr),
    settings(new SettingsDialog),
    serial(new QSerialPort)
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
    QObject::connect(&smsScheduler, &QTimer::timeout, this, &UFWLS::scheduleSMS);
    QObject::connect(ui->txtEmergencyTxtMsg, &QPlainTextEdit::textChanged, this, &UFWLS::updateCharCount);
    QObject::connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &UFWLS::editContact);
    QObject::connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &UFWLS::updateLastRowClicked);

    QObject::connect(settings, &SettingsDialog::applied, this, &UFWLS::initSerialDeviceConn);
    QObject::connect(settings, &SettingsDialog::dcntd, this, &UFWLS::xSerialDeviceConn);
//    QObject::connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &UFWLS::checkNumberFormat);
    mon.setInterval(1000);

    QFont fontTitle("Calibri",15,QFont::Bold);
    chart = new Chart(serial);
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

void UFWLS::_load(int loadingMode){
    ui->tableWidget->setSortingEnabled(false);
    QObject::disconnect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &UFWLS::updateLastRowClicked);

    QSqlQuery query;
    query.exec("SELECT contactName, contactNumber FROM contacts");
    int i = 0;
    QString lastContactNum = AddContactWindow->getContactInfo().at(1);
    // clear contents to prevent false detections during delete operations
    AddContactWindow->clearInfo();
    qDebug() << "lastContactNum: " << lastContactNum;
    while(query.next()){
        ui->tableWidget->setRowCount(i+1);
        for (int j=0; j<2; ++j){
            QTableWidgetItem* item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            item->setText(query.value(j).toString());
            qDebug() << query.value(j).toString();
            ui->tableWidget->setItem(i, j, item);

            if (query.value(j).toString() == lastContactNum){
                lastItem = ui->tableWidget->item(i, j);
                lastRow = lastItem->row();
            }
        }
        ++i;
    }
    if (i == 0) ui->tableWidget->setRowCount(0);

    ui->tableWidget->setSortingEnabled(true);
    QObject::connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &UFWLS::updateLastRowClicked);

    qDebug() << "lastRow here : " << this->lastRow;
    // loading mode
    //      default: scroll to top
    //      0 : scroll to bottom
    //      1 : scroll to last item

    switch(loadingMode){
    case 0:
        ui->tableWidget->scrollToBottom();
        ui->tableWidget->selectRow(ui->tableWidget->rowCount()-1);
        break;
    case 1:
        ui->tableWidget->selectRow(lastItem->row());
        ui->tableWidget->scrollToItem(lastItem);
        break;
    case 2:
        qDebug() << "lastRow before " << this->lastRow;
        lastRow = (lastRow >= ui->tableWidget->rowCount()) ? ui->tableWidget->rowCount()-1 : lastRow;
        ui->tableWidget->selectRow(lastRow);
        break;
    default:
        ui->tableWidget->scrollToTop();
        ui->tableWidget->selectRow(0);
    }



}

void UFWLS::initSerialDeviceConn(){
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);

    if (serial->open(QIODevice::ReadWrite)) {
        qDebug() << "Serial ok";
        alert(0,
              "Success!",
              tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl),
              this);
        _enWidgets(0);
    }
    else {
        serial->close();
        alert(2, "Open Error!", serial->errorString(), this);
    }

}

void UFWLS::xSerialDeviceConn(){
    if (serial->isOpen())
        serial->close();

    _enWidgets(1);
}

void UFWLS::handleTimeout()
{
    ui->lblAlertLevel->setText("alert level [<b>" + chart->alertLevel + "</b>]");
    ui->lblSensorReading->setText("water level [<b>" +
                                  QString::number(chart->distance,'f',2) + " m</b> "
                                  "@ <span style=\"color:#3949AB\">" + chart->strDateTime + "</span>]");
}

void UFWLS::scheduleSMS(){
    if (smsIndex >= smsBuffer.size()){
        smsScheduler.stop();
        ui->txtEmergencyTxtMsg->setEnabled(true);
        ui->lblETM->setText("<hr><b>Emergency Text Message</b><hr>");
        ui->notifyBtn->setText("Notify");
        smsBuffer.clear();
        alert(0,
              "Success!",
              "I'm done commanding the serial device to send the SMS.<br>"
              "Again, <b>I can't verify</b> if the serial device has successfully sent the SMS or not.",
              this);
        return;
    }

    SMS sms = smsBuffer.at(smsIndex++);
    QString lbl = "<hr>"
                  "<b>Emergency Text Message</b><br/>"
                  "<i>Sending to "
                  "<span style=\"color:blue\">" +(sms.contactName).left(12) + "...</span>"
                  " (" + QString::number(smsIndex) + "/" + QString::number(smsBuffer.size()) + ")</i>"
                  "<hr>";
    ui->lblETM->setText(lbl);
    QString cmd = "\r\n+SSMS: \"" + sms.contactNum + "\""
                  "\r\n" + sms.msg + "\r\n";
    serial->write(cmd.toStdString().c_str());
}

void UFWLS::updateLastRowClicked(){
    lastRow = ui->tableWidget->currentRow();
    lastItem = ui->tableWidget->currentItem();
    qDebug() << "lastRow : " << this->lastRow
             << "rowCount : " << ui->tableWidget->rowCount();
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
        _load(1);
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
                                "Contact # : <b>" + contactNo + "</b>",
                                this);

                if (res == QMessageBox::Yes){
                    dbmngr->deleteContact(contactNo);
                    _load(2);
                }
            }
        }
    }

    if (event->key() == Qt::Key_F2){
        AddContactWindow->configForAdd();
        int res = AddContactWindow->exec();
        if (res == QDialog::Accepted){
//            QList<QString> contactInfo = AddContactWindow->getContactInfo();
////            int rowCount = ui->tableWidget->rowCount();

////            ui->tableWidget->insertRow(rowCount);
////            ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(contactInfo.at(0)));
////            ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(contactInfo.at(1)));
//            dbmngr->addContact(contactInfo.at(0), contactInfo.at(1));
            _load(1);
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

void UFWLS::_enWidgets(int val){
    switch(val){
    case 0:
        ui->notifyBtn->setEnabled(true);
        ui->txtEmergencyTxtMsg->setEnabled(true);
        settings->togglecdc(false, true);
        ui->configBtn->setEnabled(true);
        break;
    case 1:
        ui->notifyBtn->setEnabled(false);
        ui->txtEmergencyTxtMsg->setEnabled(false);
        settings->togglecdc(true, false);
    }
}

void UFWLS::on_configBtn_clicked()
{
    settings->show();
}

void UFWLS::on_notifyBtn_clicked()
{
    if (ui->notifyBtn->text() == "Notify"){
        // disable editing the text message
        ui->txtEmergencyTxtMsg->setEnabled(false);
        // notify
        alert(1,
              "Warning!",
              "I am about to command the serial device to send the SMS.<br>"
              "Please be aware that <b>I can't verify</b> if the serial device has successfully sent the SMS or not!",
              this);

        QString lbl = "<hr>"
                      "<b>Emergency Text Message</b><br/>"
                      "<i>initializing ...</i>"
                      "<hr>";
        ui->lblETM->setText(lbl);

        // fill SMS_BUFFER with SMS to sent
        for (int i=0; i<ui->tableWidget->rowCount(); ++i){
            SMS sms;
            QTableWidgetItem* item;
            // contact name
            item = ui->tableWidget->item(i, 0);
            sms.contactName = item->text();
            // contact number
            item = ui->tableWidget->item(i, 1);
            sms.contactNum = item->text();
            // message
            sms.msg = ui->txtEmergencyTxtMsg->document()->toPlainText();

            smsBuffer.append(sms);
        }
        smsIndex = 0;
        smsScheduler.setInterval(3000);
        smsScheduler.start();
        ui->notifyBtn->setText("Cancel");
    }
    else if (ui->notifyBtn->text() == "Cancel"){
        int res = alert(3,
                        "Critical Operation!",
                        "Are you sure you want to cancel sending SMS commands to the serial device?",
                        this);

        if (res == QMessageBox::Yes){
            smsScheduler.stop();
            ui->txtEmergencyTxtMsg->setEnabled(true);
            ui->lblETM->setText("<hr><b>Emergency Text Message</b><hr>");
            ui->notifyBtn->setText("Notify");
            smsBuffer.clear();
        }
    }
}
