/**************************************************************************************
** Filename    : dialogviewtasklist.h
** Author      : Emile
** Purpose     : Header file for dialogviewtasklist.cpp. Contains a screen that shows
**               scheduler tasks, both from the PC-program and from the EBrew hardware.
** License     : This is free software: you can redistribute it and/or modify
**               it under the terms of the GNU General Public License as published by
**               the Free Software Foundation, either version 3 of the License, or
**               (at your option) any later version.
**
**               This file is distributed in the hope that it will be useful,
**               but WITHOUT ANY WARRANTY; without even the implied warranty of
**               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**               GNU General Public License for more details.
**
**               You should have received a copy of the GNU General Public License
**               along with this file.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
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
