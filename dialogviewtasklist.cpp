#include "dialogviewtasklist.h"
#include "ui_dialogviewtasklist.h"

DialogViewTaskList::DialogViewTaskList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogViewTaskList)
{
    ui->setupUi(this);
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->teTaskList->setText(pEbrew->schedulerEbrew->list_all_tasks());
}

DialogViewTaskList::~DialogViewTaskList()
{
    delete ui;
}
