#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QList>
#include <QString>
#include <QDebug>

#include "alert.hpp"

// a database manager (not so obvious!)
// * handles the connection and modification of the database
class DbManager
{
public:
    // opens a connection to a database
    int open(const QString& path);
    // adds a contact into the database
    bool addContact(const QString& cname,
                   const QString& cnum);
    // removes an existing record of a student from the database
    bool deleteContact(const QString& cnum);

    bool updateContact(const QString& cname, const QString& cnum);

    QList<QString> contactExists(const QString& cname, const QString& cnum);

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_HPP
