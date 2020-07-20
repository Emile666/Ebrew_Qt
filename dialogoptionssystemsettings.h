#ifndef DIALOGOPTIONSSYSTEMSETTINGS_H
#define DIALOGOPTIONSSYSTEMSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogOptionsSystemSettings;
}

class DialogOptionsSystemSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsSystemSettings(QWidget *parent = nullptr);
    ~DialogOptionsSystemSettings();

private slots:
    void on_buttonBox_accepted();
    void on_rb1_clicked();
    void on_rb2_clicked();
    void on_rb3_clicked();

    void on_cbCommCh_currentIndexChanged(int index);

private:
    Ui::DialogOptionsSystemSettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
    void set_mode0(void);
    void set_mode1(void);
    void set_mode2(void);
    void set_mode3(void);
};

#endif // DIALOGOPTIONSSYSTEMSETTINGS_H
