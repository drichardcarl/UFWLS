#ifndef ALERT_HPP
#define ALERT_HPP

#include <QString>
#include <QMessageBox>

// general-purpose information display
int alert(int type, QString title, QString msg, QWidget* parent=0);

#endif // ALERT_HPP

