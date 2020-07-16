#ifndef DIALOGBREWDAYSETTINGS_H
#define DIALOGBREWDAYSETTINGS_H

#include <QDialog>

namespace Ui {
class DialogBrewDaySettings;
}

class DialogBrewDaySettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBrewDaySettings(QWidget *parent = nullptr);
    ~DialogBrewDaySettings();

private:
    Ui::DialogBrewDaySettings *ui;
};

#endif // DIALOGBREWDAYSETTINGS_H
