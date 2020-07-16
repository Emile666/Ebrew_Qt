#ifndef DIALOGEDITFIXPARAMETERS_H
#define DIALOGEDITFIXPARAMETERS_H

#include <QDialog>

namespace Ui {
class dialogeditfixparameters;
}

class dialogeditfixparameters : public QDialog
{
    Q_OBJECT

public:
    explicit dialogeditfixparameters(QWidget *parent = nullptr);
    ~dialogeditfixparameters();

private:
    Ui::dialogeditfixparameters *ui;
};

#endif // DIALOGEDITFIXPARAMETERS_H
