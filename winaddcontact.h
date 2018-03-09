#ifndef WINADDCONTACT_H
#define WINADDCONTACT_H

#include <QDialog>
#include <QRegExp>
#include <QDebug>

#include "alert.hpp"
#include "dbmanager.hpp"

namespace Ui {
class WinAddContact;
}

class WinAddContact : public QDialog
{
    Q_OBJECT

public:
    explicit WinAddContact(QWidget *parent = 0);
    ~WinAddContact();

    QList<QString> getContactInfo();
    void setDbManager(DbManager* dbmngr);
    void configForEdit(const QString& cname, const QString& cnum);
    void configForAdd();

protected:
    void showEvent(QShowEvent* e);

private:
    Ui::WinAddContact *ui;
    QString txtContactName;
    QString txtContactNum;
    DbManager *dbmngr;
    int mode;

private slots:
    void on_BtnAdd_clicked();
    void on_BtnCancel_clicked();
};

#endif // WINADDCONTACT_H
