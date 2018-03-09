#include "dbmanager.hpp"

// * alerts the user when the database is not found on the current directory
// * opens the database if it is present in the current directory
int DbManager::open(const QString& path){
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    if (!QFile::exists(path))
    {
         alert(2,
               "Failure to connect to database",
               "Unable to establish a database connection.<br>"
               "Make sure that the <b>*.db</b> file is in the following location:<br><b>" +
               QDir::currentPath() + "</b>.");

         return -1;
    }
    else
    {
       m_db.open();
       return 0;
    }
}

bool DbManager::addContact(const QString& cname,
                           const QString& cnum)
{
   bool success = false;
   QSqlQuery query;
   query.prepare("INSERT INTO "
                 "contacts (contactName, contactNumber) "
                 "VALUES (?, ?)");
   query.addBindValue(cname);
   query.addBindValue(cnum);

   if(query.exec())
   {
       success = true;
   }
   else
   {
        qDebug() << "addStudent error:  "
                 << query.lastError();
   }

   return success;
}


bool DbManager::deleteContact(const QString& cnum)
{
   bool success = false;
   QSqlQuery query;
   query.prepare("DELETE FROM contacts "
                 "WHERE contactNumber=?");
   query.addBindValue(cnum);

   if(query.exec())
   {
       success = true;
   }
   else
   {
        qDebug() << "deleteStudent error:  "
                 << query.lastError();
   }

   return success;
}

bool DbManager::updateContact(const QString& cname, const QString& cnum){
    bool success = false;
    QSqlQuery query;
    query.prepare("UPDATE contacts "
                  "SET contactName=? "
                  "WHERE contactNumber=?");
    qDebug() << "before bind!";
    query.addBindValue(cname);
    qDebug() << "before bin2!";
    query.addBindValue(cnum);
    qDebug() << "before exec!";

    if(query.exec())
    {
        success = true;
        qDebug() << "inside exec!";
    }
    else
    {
         qDebug() << "updateStudent error:  "
                  << query.lastError();
    }

    return success;
}

QList<QString> DbManager::contactExists(const QString &cname, const QString &cnum){
    QList<QString> existingContact;
    QSqlQuery query;
    query.prepare("SELECT contactName, contactNumber FROM contacts "
                  "WHERE contactNumber=?");
    query.addBindValue(cnum);

    query.exec();

    // there must be only one match for this since a contact number
    // must only be associated to at most 1 person
    while (query.next()) {
        existingContact.append(query.value(0).toString());
        existingContact.append(query.value(1).toString());
    }

    return existingContact;
}
