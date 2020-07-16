#ifndef DIALOGEDITMASHSCHEME_H
#define DIALOGEDITMASHSCHEME_H

#include <QDialog>

namespace Ui {
class DialogEditMashScheme;
}

class DialogEditMashScheme : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditMashScheme(QWidget *parent = nullptr);
    ~DialogEditMashScheme();

private:
    Ui::DialogEditMashScheme *ui;
};

#endif // DIALOGEDITMASHSCHEME_H
