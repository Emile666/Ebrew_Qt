#ifndef DIALOGOPTIONSPIDSETTINGS_H
#define DIALOGOPTIONSPIDSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogOptionsPidSettings;
}

class DialogOptionsPidSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsPidSettings(QWidget *parent = nullptr);
    ~DialogOptionsPidSettings();

private slots:
    void on_buttonBox_accepted();

    void on_cbDStart_stateChanged(int arg1);

private:
    Ui::DialogOptionsPidSettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGOPTIONSPIDSETTINGS_H
