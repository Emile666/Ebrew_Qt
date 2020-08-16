/**************************************************************************************
** Filename    : dialogeditfixparameters.h
** Author      : Emile
** Purpose     : Header file for dialogeditfixparameters.cpp. Contains a dialog screen
**               with which to set variables to a fixed value.
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
#ifndef DIALOGEDITFIXPARAMETERS_H
#define DIALOGEDITFIXPARAMETERS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogEditFixParameters;
}

class DialogEditFixParameters : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditFixParameters(QWidget *parent = nullptr);
    ~DialogEditFixParameters();
    void set_switch_fix(void);

private slots:
    void on_cbHltSp_stateChanged(int arg1);
    void on_cbHltTemp_stateChanged(int arg1);
    void on_cbHltPid_stateChanged(int arg1);
    void on_cbHltVol_stateChanged(int arg1);
    void on_cbMltTemp_stateChanged(int arg1);
    void on_cbMltVol_stateChanged(int arg1);
    void on_cbBkSp_stateChanged(int arg1);
    void on_cbBkTemp_stateChanged(int arg1);
    void on_cbBkVol_stateChanged(int arg1);
    void on_cbBkPid_stateChanged(int arg1);
    void on_cbTTriac_stateChanged(int arg1);
    void on_cbMsIdx_stateChanged(int arg1);
    void on_cbSpIdx_stateChanged(int arg1);
    void on_cbStd_stateChanged(int arg1);
    void on_buttonBox_accepted();
    void on_pbApply_clicked();
    void on_buttonBox_rejected();

private:
    Ui::DialogEditFixParameters *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGEDITFIXPARAMETERS_H
