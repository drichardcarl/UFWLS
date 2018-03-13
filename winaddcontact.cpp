#include "winaddcontact.h"
#include "ui_winaddcontact.h"

WinAddContact::WinAddContact(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WinAddContact),
    dbmngr(nullptr),
    mode(0)
{
    ui->setupUi(this);
}

WinAddContact::~WinAddContact()
{
    delete ui;
}

void WinAddContact::showEvent(QShowEvent* e){
    ui->BtnAdd->setText(
        (mode) ? "Save" : "Add"
    );

    ui->TxtContactNumber->setEnabled(
        (mode) ? false : true
    );

    ui->TxtContactName->setFocus();
}

void WinAddContact::on_BtnAdd_clicked()
{
    txtContactName = ui->TxtContactName->text();
    txtContactNum = ui->TxtContactNumber->text();

    if (txtContactName == ""){
        alert(1, "Invalid Field", "Please enter a valid <b>Contact Name</b>!");
        return;
    }

    if (txtContactNum.size() != 15){
        alert(1, "Invalid Field", "Please enter a valid <b>Contact Number</b>!");
        return;
    }

    txtContactNum.replace("(","");
    txtContactNum.replace(")","");

    if (mode == 1){
        bool succ = dbmngr->updateContact(txtContactName, txtContactNum);
        if (succ) qDebug() << "update successful!";
    }
    else{
        QList<QString> existingContact = dbmngr->contactExists(txtContactName, txtContactNum);
        qDebug() << "existing contact : " << existingContact;
        if (existingContact.size() == 2){
            alert(2,
                  "Contact Exists!",
                  "The emergency contact number specified (<b>" + existingContact.at(1) + "</b>) "
                  "is already associated to emergency contact person "
                  "<b>" + existingContact.at(0) + "</b>!<br><br>"
                  "Please choose another contact number.");
            return;
        }
        dbmngr->addContact(txtContactName, txtContactNum);
    }

    ui->TxtContactName->clear();
    ui->TxtContactNumber->clear();

    this->accept();
}

void WinAddContact::on_BtnCancel_clicked()
{
    this->reject();
}

QList<QString> WinAddContact::getContactInfo(){
    QList<QString> contactInfo;
    contactInfo.append(txtContactName);
    contactInfo.append(txtContactNum);

    return contactInfo;
}

void WinAddContact::setDbManager(DbManager* dbmngr){
    this->dbmngr = dbmngr;
}

void WinAddContact::configForEdit(const QString& cname, const QString& cnum){
    this->mode = 1;
    ui->TxtContactName->setText(cname);
    ui->TxtContactNumber->setText(cnum);
}

void WinAddContact::configForAdd(){
    this->mode = 0;
    ui->TxtContactName->clear();
    ui->TxtContactNumber->clear();
}

void WinAddContact::clearInfo(){
    txtContactName = "";
    txtContactNum = "";
}
