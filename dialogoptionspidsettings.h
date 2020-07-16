#ifndef DIALOGOPTIONSPIDSETTINGS_H
#define DIALOGOPTIONSPIDSETTINGS_H

#include <QDialog>

namespace Ui {
class DialogOptionsPidSettings;
}

class DialogOptionsPidSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsPidSettings(QWidget *parent = nullptr);
    ~DialogOptionsPidSettings();

private:
    Ui::DialogOptionsPidSettings *ui;
};

#endif // DIALOGOPTIONSPIDSETTINGS_H
