#include "alert.hpp"

int alert(int type, QString title, QString msg, QWidget* parent){
    switch (type) {
    // information
    case 0:
        return QMessageBox::information(parent, title, msg);
        break;
    // warning
    case 1:
        return QMessageBox::warning(parent, title, msg);
        break;
    // critical
    case 2:
        return QMessageBox::critical(parent, title, msg);
        break;
    // critical with custom buttons
    case 3:
        return QMessageBox::critical(parent, title, msg, QMessageBox::Yes, QMessageBox::No);
        break;
    // critical with custom buttons 2
    case 4:
        return QMessageBox::critical(parent, title, msg, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
        break;
    }
}
