/**************************************************************************************
** Filename    : dialogviewprogress.h
** Author      : Emile
** Purpose     : Header file for dialogviewprogress.cpp. Contains an information screen
**               for tracking status of mashing, sparging and boiling.
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
#ifndef DIALOGVIEWPROGRESS_H
#define DIALOGVIEWPROGRESS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogViewProgress;
}

static const QString   MashTitle("  Mash Temp Time  PreHt Timer Timer        Time-\n"
                                 "  idx  [°C] [sec] [sec] [sec] Status       Stamp\n"
                                 "--------------------------------------------------");
static const QString SpargeTitle("index        From HLT to MLT  | From MLT to Boil-kettle\n"
                                 "------------------------------|--------------------------");

class DialogViewProgress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogViewProgress(QWidget *parent = nullptr);
    ~DialogViewProgress();

public slots:
    void onUpdateProgress(void);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    Ui::DialogViewProgress *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
    int        timerId;
};

#endif // DIALOGVIEWPROGRESS_H
