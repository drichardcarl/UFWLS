#-------------------------------------------------
#
# Project created by QtCreator 2018-03-05T21:50:43
#
#-------------------------------------------------

QT       += core gui charts sql serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UFWLS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        ufwls.cpp \
    chart.cpp \
    alert.cpp \
    winaddcontact.cpp \
    dbmanager.cpp \
    settingsdialog.cpp \
    about.cpp

HEADERS += \
        ufwls.h \
    chart.h \
    alert.hpp \
    winaddcontact.h \
    dbmanager.hpp \
    settingsdialog.h \
    about.h

FORMS += \
        ufwls.ui \
    winaddcontact.ui \
    settingsdialog.ui \
    about.ui

RESOURCES += \
    ufwls.qrc

win32:RC_ICONS += ufwls.ico
