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

#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QDateTime>
#include <QTime>

#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
//#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include <QDateTimeAxis>
#include <QSystemTrayIcon>
#include <QSerialPort>
#include <QByteArray>

#include "alert.hpp"

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

//![1]
class Chart: public QChart
{
    Q_OBJECT
public:
    Chart(QSerialPort* serial, QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~Chart();

    void setTrayIcon(QSystemTrayIcon* trayIcon);

    QString alertLevel = "NO READING";
    QString strDateTime = "00/00/0000 00:00:00";
    qreal distance = 0.00;
    bool initDevice = false;

public slots:
    void handleTimeout();
    void toggleTrayMsg();

private:
    QTimer watcher;
    QTimer trayMsgMngr;
//    QRandomGenerator random;
    QLineSeries *data;
    QString title;
    QDateTimeAxis *xAxis;
    QValueAxis *yAxis;
    qint64 xval;
    qreal yval;

    QSystemTrayIcon* trayIcon;
    QString alertStatus;

    QSerialPort* serial;
    bool initSerial = false;
    QDateTime prevTime;
    bool noReading = false;
    int cNoReading = 0;

};
//![1]

#endif /* CHART_H */
