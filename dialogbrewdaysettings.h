#ifndef DIALOGBREWDAYSETTINGS_H
#define DIALOGBREWDAYSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogBrewDaySettings;
}

class DialogBrewDaySettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBrewDaySettings(QWidget *parent = nullptr);
    ~DialogBrewDaySettings();

private slots:
    void on_buttonBox_accepted();

    void on_cbMash1_stateChanged(int arg1);

private:
    Ui::DialogBrewDaySettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGBREWDAYSETTINGS_H
