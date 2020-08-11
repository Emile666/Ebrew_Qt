#ifndef DIALOGVIEWTASKLIST_H
#define DIALOGVIEWTASKLIST_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogViewTaskList;
}

class DialogViewTaskList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogViewTaskList(QWidget *parent = nullptr);
    ~DialogViewTaskList();
    void     readTaskListData(void);

private slots:
    void on_refreshButton_clicked();

private:
    Ui::DialogViewTaskList *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGVIEWTASKLIST_H
