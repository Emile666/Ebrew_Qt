/**************************************************************************************
** Filename    : dialogoptionspidsettings.h
** Author      : Emile
** Purpose     : Header file for dialogoptionspidsettings.cpp. Contains a dialog screen
**               with which to change parameters for the PID-controller.
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
#ifndef DIALOGOPTIONSPIDSETTINGS_H
#define DIALOGOPTIONSPIDSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogOptionsPidSettings;
}

class DialogOptionsPidSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsPidSettings(QWidget *parent = nullptr);
    ~DialogOptionsPidSettings();

private slots:
    void on_buttonBox_accepted();

    void on_cbDStart_stateChanged(int arg1);

private:
    Ui::DialogOptionsPidSettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGOPTIONSPIDSETTINGS_H
