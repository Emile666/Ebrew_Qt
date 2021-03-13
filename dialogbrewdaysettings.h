/**************************************************************************************
** Filename    : dialogbrewdaysettings.h
** Author      : Emile
** Purpose     : Header file for dialogbrewdaysettings.cpp. Contains a dialog screen
**               for changing mashing, sparging, boiling and CIP parameters.
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
#ifndef DIALOGBREWDAYSETTINGS_H
#define DIALOGBREWDAYSETTINGS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogBrewDaySettings;
}

class DialogBrewDaySettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBrewDaySettings(QWidget *parent = nullptr);
    ~DialogBrewDaySettings();

private slots:
    void on_buttonBox_accepted();
    void on_cbMash1_stateChanged(int arg1);

    void on_cbBKrecirc_stateChanged(int arg1);

    void on_cbMash2_stateChanged(int arg1);

private:
    Ui::DialogBrewDaySettings *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGBREWDAYSETTINGS_H
