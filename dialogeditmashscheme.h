#ifndef DIALOGEDITMASHSCHEME_H
#define DIALOGEDITMASHSCHEME_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogEditMashScheme;
}

class DialogEditMashScheme : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditMashScheme(QWidget *parent = nullptr);
    ~DialogEditMashScheme();

private slots:
    void on_pushButton_clicked();   // Load Mash Scheme File...
    void on_pushButton_2_clicked(); // Save As...
    void on_pushButton_3_clicked(); // Save

private:
    Ui::DialogEditMashScheme *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGEDITMASHSCHEME_H
