/**************************************************************************************
** Filename    : dialogoptionssystemsettings.h
** Author      : Emile
** Purpose     : Header file for dialogoptionssystemsettings.cpp. Contains a dialog screen
**               with which to change system-wide parameters, such as communications.
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
#ifndef DIALOGOPTIONSSYSTEMSETTINGS_H
#define DIALOGOPTIONSSYSTEMSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogOptionsSystemSettings;
}

class DialogOptionsSystemSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptionsSystemSettings(QWidget *parent = nullptr);
    ~DialogOptionsSystemSettings();
    int     commChannel;
    QString commSettings;
    QString commUdp;
    int     commCb;

private slots:
    void on_buttonBox_accepted();
    void on_cbCommCh_currentIndexChanged(int index);

private:
    Ui::DialogOptionsSystemSettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGOPTIONSSYSTEMSETTINGS_H
