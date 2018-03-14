/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chart.h"
#include <QCategoryAxis>

Chart::Chart(QSerialPort* serial, QGraphicsItem *parent, Qt::WindowFlags wFlags):
    serial(serial),
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    xAxis(new QDateTimeAxis),
    xval(0),
    yval(0),
    trayIcon(nullptr)
{
    QObject::connect(&watcher, &QTimer::timeout, this, &Chart::handleTimeout);
    watcher.setInterval(5000);

    QObject::connect(&trayMsgMngr, &QTimer::timeout, this, &Chart::toggleTrayMsg);
    trayMsgMngr.setInterval(30000);
//    random.seed();
    data = new QSplineSeries(this);
    QPen green(Qt::red);
    green.setWidth(3);
    data->setPen(green);

    QDateTime xValue;
//    xValue.setDate(QDate(2012, 1 , 18));
//    xValue.setTime(QTime(9, 34));
    xValue.setDate(QDate::currentDate());
    xValue.setTime(QTime::currentTime());
    xval = xValue.toMSecsSinceEpoch();
    yval = 3;
    data->append(xval, yval);

//    xValue.setDate(QDate::currentDate());
//    xValue.setTime(QTime::currentTime());
//    qDebug() << xValue.date() << " _ " << xValue.time();
//    yValue = 5;
//    data->append(xValue.toMSecsSinceEpoch(), yValue);
    QFont fontAxis("Calibri",13,QFont::Bold);

    addSeries(data);
    createDefaultAxes();
    xAxis = new QDateTimeAxis;
    xAxis->setFormat("MM/dd/yyyy<br>HH:mm:ss");
    setAxisX(xAxis, data);
    xAxis->setTickCount(5);
    xAxis->setTitleFont(fontAxis);
    xAxis->setTitleText("Date/Time");

    yAxis = new QValueAxis;
    yAxis->setRange(0,4);
    yAxis->setTickCount(7);
    yAxis->setTitleFont(fontAxis);
    yAxis->setTitleText("Water Level [m]");
    setAxisY(yAxis, data);

//    axisY()->setRange(0,4);
//    axisY()->setTitleFont(fontAxis);
//    axisY()->setTitleText("Water Distance from Sensor [m]");

    QDateTime now = QDateTime(QDate::currentDate(), QTime::currentTime());
    QDateTime rangeFrom = QDateTime::fromMSecsSinceEpoch(now.toMSecsSinceEpoch()-7500);
    QDateTime rangeTo = QDateTime::fromMSecsSinceEpoch(now.toMSecsSinceEpoch()+7500);
    xAxis->setRange(rangeFrom,rangeTo);
    prevTime = now;

    QCategoryAxis *axisY3 = new QCategoryAxis;
    axisY3->append("<span style=\"color:#388E3C\"><b>Normal</b></span>", 1.33);
    axisY3->append("<span style=\"color:#F57C00\"><b>Warning</b></span>", 2.66);
    axisY3->append("<span style=\"color:#D32F2F\"><b>Evacuate</b></span>", 3.99);
//    axisY3->setLinePenColor(data->pen().color());
    QPen pen(QColor("#E57373"));
    pen.setStyle(Qt::DashDotDotLine);
    axisY3->setGridLinePen(pen);
    axisY3->setTitleFont(fontAxis);
    axisY3->setTitleText("Alert Level");

    addAxis(axisY3, Qt::AlignRight);
    data->attachAxis(axisY3);

    watcher.start();
    trayMsgMngr.start();
    qDebug() << xval << " ; " << yval;
}

Chart::~Chart()
{

}

void Chart::handleTimeout()
{
    if (!serial->isOpen()) {
        initSerial = false;
        return;
    }
    if (!initSerial){
        if (serial->bytesAvailable()){
            serial->readAll();
            return;
        }
        else
            initSerial = true;
    }

    char buffer[250];
    serial->readLine(buffer, 250);
    if (QString(buffer).size() == 0){
        noReading = true;
        return;
    }

    qreal h = plotArea().height();
    qreal w = plotArea().width();
    int xtc = ceil(w/(h/6));
    qDebug() << xtc;
    xAxis->setTickCount(xtc);
    qreal x = plotArea().width()/15000;

    QDateTime dateTime;
    dateTime.setDate(QDate::currentDate());
    dateTime.setTime(QTime::currentTime());
    xval = dateTime.toMSecsSinceEpoch();
    strDateTime = dateTime.toString("MM/dd/yyyy HH:mm:ss");

    qint64 nowMs = dateTime.toMSecsSinceEpoch();
    qint64 prevMs = prevTime.toMSecsSinceEpoch();
    qint64 diff = nowMs-prevMs;

    qDebug() << "now : " << nowMs
             << "prev : " << prevMs
             << "diff : " << diff;

    prevTime = dateTime;

    yval = QString(buffer).toDouble();
    distance = 4 - yval; // base height - sensor distance
    yval = distance;

    qDebug() << "serial buffer:" << buffer
             << "yval: " << yval
             << "distance: " << distance;

    QColor color;
    QPen pen;
    pen.setWidth(3);
    if (yval >= 2.66){
        color.setNamedColor("#D32F2F");
        pen.setColor(color);
        data->setPen(pen);
        alertLevel = "<span style=\"color:#D32F2F\"><b>EVACUATE</b></span>";
        alertStatus = "Evacuate";
    }
    else if (yval >= 1.33 && yval < 2.66){
        color.setNamedColor("#F57C00");
        pen.setColor(color);
        data->setPen(pen);
        alertLevel = "<span style=\"color:#F57C00\"><b>WARNING</b></span>";
        alertStatus = "Warning";
    }
    else {
        color.setNamedColor("#388E3C");
        pen.setColor(color);
        data->setPen(pen);
        alertLevel = "<span style=\"color:#388E3C\"><b>NORMAL</b></span>";
        alertStatus = "Normal";

    }
    data->append(xval, yval);
    scroll(diff*x, 0);
}

void Chart::setTrayIcon(QSystemTrayIcon* trayIcon){
    this->trayIcon = trayIcon;
}

void Chart::toggleTrayMsg(){
    if (noReading) return;
    if (alertStatus == "Warning"){
        trayIcon->showMessage("Ultrasonic Flood Water Level Sensor (UFWLS)",
                              strDateTime + "\n"
                              "Please take precaution.\n"
                              "Water level is at " + QString::number(distance,'f',2) + " m.\n"
                              "Alert status is raised to Warning!",
                              QSystemTrayIcon::Warning);
    }
    else if (alertStatus == "Evacuate"){
        trayIcon->showMessage("Ultrasonic Flood Water Level Sensor (UFWLS)",
                              strDateTime + "\n"
                              "Water is at a critical level (" +
                              QString::number(distance,'f',2) + " m)!\n"
                              "Alert status is raised to Evacuation!",
                              QSystemTrayIcon::Critical);
    }

}
