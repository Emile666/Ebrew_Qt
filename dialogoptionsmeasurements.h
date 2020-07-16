#ifndef DIALOGOPTIONSMEASUREMENTS_H
#define DIALOGOPTIONSMEASUREMENTS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogOptionsMeasurements;
}

class DialogOptionsMeasurements : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsMeasurements(QWidget *parent = nullptr);
    ~DialogOptionsMeasurements();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DialogOptionsMeasurements *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGOPTIONSMEASUREMENTS_H
