/**************************************************************************************
** Filename    : dialogviewstatusalarms.h
** Author      : Emile
** Purpose     : Header file for dialogviewstatusalarms.cpp. Contains an information screen
**               for displaying various status and alarm signals.
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
#ifndef DIALOGVIEWSTATUSALARMS_H
#define DIALOGVIEWSTATUSALARMS_H
#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogViewStatusAlarms;
}

class DialogViewStatusAlarms : public QDialog
{
    Q_OBJECT

public:
    explicit DialogViewStatusAlarms(QWidget *parent = nullptr);
    ~DialogViewStatusAlarms();

public slots:
    void onUpdateProgress(void);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    Ui::DialogViewStatusAlarms *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
    int        timerId;
};

#endif // DIALOGVIEWSTATUSALARMS_H
