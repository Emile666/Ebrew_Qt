#ifndef DIALOGOPTIONSGUISETTINGS_H
#define DIALOGOPTIONSGUISETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class dialogoptionsguisettings;
}

class dialogoptionsguisettings : public QDialog
{
    Q_OBJECT

public:
    explicit dialogoptionsguisettings(QWidget *parent = nullptr);
    ~dialogoptionsguisettings();

private slots:
    void on_buttonBox_accepted();
    void on_pbColInOff_clicked();
    void on_pbColInOn_clicked();
    void on_pbColOutOff_clicked();
    void on_pbColOutOn_clicked();

private:
    Ui::dialogoptionsguisettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)

    void updateButtonColor(QPushButton *button, const QColor &color);
};

#endif // DIALOGOPTIONSGUISETTINGS_H
