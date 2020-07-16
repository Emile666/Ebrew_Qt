#ifndef DIALOGOPTIONSMEASUREMENTS_H
#define DIALOGOPTIONSMEASUREMENTS_H

#include <QDialog>

namespace Ui {
class DialogOptionsMeasurements;
}

class DialogOptionsMeasurements : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsMeasurements(QWidget *parent = nullptr);
    ~DialogOptionsMeasurements();

private:
    Ui::DialogOptionsMeasurements *ui;
};

#endif // DIALOGOPTIONSMEASUREMENTS_H
