/**************************************************************************************
** Filename    : dialogeditfixparameters.cpp
** Author      : Emile
** Purpose     : This file contains a dialog screen with which to set variables to a
**               fixed value. Foever every variable, it uses a switch and a fix variable.
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
#include "dialogeditfixparameters.h"
#include "ui_dialogeditfixparameters.h"

DialogEditFixParameters::DialogEditFixParameters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditFixParameters)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);
    ui->cbHltSp->setChecked(pEbrew->tset_hlt_sw);
    ui->sbHltSp->setValue(pEbrew->tset_hlt_fx);
    ui->cbHltTemp->setChecked(pEbrew->thlt_sw);
    ui->sbHltTemp->setValue(pEbrew->thlt_fx);
    ui->cbHltVol->setChecked(pEbrew->vhlt_sw);
    ui->sbHltVol->setValue(pEbrew->vhlt_fx);
    ui->cbHltPid->setChecked(pEbrew->gamma_hlt_sw);
    ui->sbHltPid->setValue(pEbrew->gamma_hlt_fx);

    ui->cbMltTemp->setChecked(pEbrew->tmlt_sw);
    ui->sbMltTemp->setValue(pEbrew->tmlt_fx);
    ui->cbMltVol->setChecked(pEbrew->vmlt_sw);
    ui->sbMltVol->setValue(pEbrew->vmlt_fx);

    ui->cbBkSp->setChecked(pEbrew->tset_boil_sw);
    ui->sbBkSp->setValue(pEbrew->tset_boil_fx);
    ui->cbBkTemp->setChecked(pEbrew->tboil_sw);
    ui->sbBkTemp->setValue(pEbrew->tboil_fx);
    ui->cbBkVol->setChecked(pEbrew->vboil_sw);
    ui->sbBkVol->setValue(pEbrew->vboil_fx);
    ui->cbBkPid->setChecked(pEbrew->gamma_boil_sw);
    ui->sbBkPid->setValue(pEbrew->gamma_boil_fx);
} // DialogEditFixParameters::DialogEditFixParameters()

DialogEditFixParameters::~DialogEditFixParameters()
{
    delete ui;
} // DialogEditFixParameters::~DialogEditFixParameters()

//---------------------------------------------------------------
// Hot Liquid Tun (HLT)
//---------------------------------------------------------------
void DialogEditFixParameters::on_cbHltSp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbHltSp->setEnabled(true);
        ui->sbHltSp->setFocus();
        ui->sbHltSp->selectAll();
    } // if
    else
    {
        ui->sbHltSp->setEnabled(false);
        ui->cbHltSp->setFocus();
    } // else
} // DialogEditFixParameters::on_cbHltSp_stateChanged()

void DialogEditFixParameters::on_cbHltTemp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbHltTemp->setEnabled(true);
        ui->sbHltTemp->setFocus();
        ui->sbHltTemp->selectAll();
    } // if
    else
    {
        ui->sbHltTemp->setEnabled(false);
        ui->cbHltTemp->setFocus();
    } // else
} // DialogEditFixParameters::on_cbHltTemp_stateChanged()

void DialogEditFixParameters::on_cbHltVol_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbHltVol->setEnabled(true);
        ui->sbHltVol->setFocus();
        ui->sbHltVol->selectAll();
    } // if
    else
    {
        ui->sbHltVol->setEnabled(false);
        ui->cbHltVol->setFocus();
    } // else
} // DialogEditFixParameters::on_cbHltVol_stateChanged()

void DialogEditFixParameters::on_cbHltPid_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbHltPid->setEnabled(true);
        ui->sbHltPid->setFocus();
        ui->sbHltPid->selectAll();
    } // if
    else
    {
        ui->sbHltPid->setEnabled(false);
        ui->cbHltPid->setFocus();
    } // else
} // DialogEditFixParameters::on_cbHltPid_stateChanged()

//---------------------------------------------------------------
// Mash Lauter Tun (MLT)
//---------------------------------------------------------------
void DialogEditFixParameters::on_cbMltTemp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbMltTemp->setEnabled(true);
        ui->sbMltTemp->setFocus();
        ui->sbMltTemp->selectAll();
    } // if
    else
    {
        ui->sbMltTemp->setEnabled(false);
        ui->cbMltTemp->setFocus();
    } // else
} // DialogEditFixParameters::on_cbMltTemp_stateChanged()

void DialogEditFixParameters::on_cbMltVol_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbMltVol->setEnabled(true);
        ui->sbMltVol->setFocus();
        ui->sbMltVol->selectAll();
    } // if
    else
    {
        ui->sbMltVol->setEnabled(false);
        ui->cbMltVol->setFocus();
    } // else
} // DialogEditFixParameters::on_cbMltVol_stateChanged()

//---------------------------------------------------------------
// Boil Kettle
//---------------------------------------------------------------
void DialogEditFixParameters::on_cbBkSp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbBkSp->setEnabled(true);
        ui->sbBkSp->setFocus();
        ui->sbBkSp->selectAll();
    } // if
    else
    {
        ui->sbBkSp->setEnabled(false);
        ui->cbBkSp->setFocus();
    } // else
} // DialogEditFixParameters::on_cbBkSp_stateChanged()

void DialogEditFixParameters::on_cbBkTemp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbBkTemp->setEnabled(true);
        ui->sbBkTemp->setFocus();
        ui->sbBkTemp->selectAll();
    } // if
    else
    {
        ui->sbBkTemp->setEnabled(false);
        ui->cbBkTemp->setFocus();
    } // else
} // DialogEditFixParameters::on_cbBkTemp_stateChanged()

void DialogEditFixParameters::on_cbBkVol_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbBkVol->setEnabled(true);
        ui->sbBkVol->setFocus();
        ui->sbBkVol->selectAll();
    } // if
    else
    {
        ui->sbBkVol->setEnabled(false);
        ui->cbBkVol->setFocus();
    } // else
} // DialogEditFixParameters::on_cbBkVol_stateChanged()

void DialogEditFixParameters::on_cbBkPid_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbBkPid->setEnabled(true);
        ui->sbBkPid->setFocus();
        ui->sbBkPid->selectAll();
    } // if
    else
    {
        ui->sbBkPid->setEnabled(false);
        ui->cbBkPid->setFocus();
    } // else
} // DialogEditFixParameters::on_cbBkPid_stateChanged()

//---------------------------------------------------------------
// Other
//---------------------------------------------------------------
void DialogEditFixParameters::on_cbTTriac_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbTTriac->setEnabled(true);
        ui->sbTTriac->setFocus();
        ui->sbTTriac->selectAll();
    } // if
    else
    {
        ui->sbTTriac->setEnabled(false);
        ui->cbTTriac->setFocus();
    } // else
} // DialogEditFixParameters::on_cbTTriac_stateChanged()

void DialogEditFixParameters::on_cbMsIdx_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbMsIdx->setEnabled(true);
        ui->sbMsIdx->setFocus();
        ui->sbMsIdx->selectAll();
    } // if
    else
    {
        ui->sbMsIdx->setEnabled(false);
        ui->cbMsIdx->setFocus();
    } // else
} // DialogEditFixParameters::on_cbMsIdx_stateChanged()

void DialogEditFixParameters::on_cbSpIdx_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbSpIdx->setEnabled(true);
        ui->sbSpIdx->setFocus();
        ui->sbSpIdx->selectAll();
    } // if
    else
    {
        ui->sbSpIdx->setEnabled(false);
        ui->cbSpIdx->setFocus();
    } // else
} // DialogEditFixParameters::on_cbSpIdx_stateChanged()

void DialogEditFixParameters::on_cbStd_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->comboStd->setEnabled(true);
        ui->comboStd->setFocus();
    } // if
    else
    {
        ui->comboStd->setEnabled(false);
        ui->cbStd->setFocus();
    } // else
} // DialogEditFixParameters::on_cbStd_stateChanged()
//---------------------------------------------------------------

void DialogEditFixParameters::on_buttonBox_accepted()
{
    set_switch_fix(); // Set SW & Fix and leave dialog
} // DialogEditFixParameters::on_buttonBox_accepted()

void DialogEditFixParameters::on_pbApply_clicked()
{
    set_switch_fix(); // Set SW & Fix without leaving dialog
} // DialogEditFixParameters::on_pbApply_clicked()

void DialogEditFixParameters::on_buttonBox_rejected()
{
    //----------------------
    // HLT
    //----------------------
    pEbrew->tset_hlt_sw  = false;
    pEbrew->tset_hlt_fx  = 0.0;
    pEbrew->thlt_sw      = false;
    pEbrew->thlt_fx      = 0.0;
    pEbrew->vhlt_sw      = false;
    pEbrew->vhlt_fx      = 0.0;
    pEbrew->gamma_hlt_sw = false;
    pEbrew->gamma_hlt_fx = 0.0;
    //----------------------
    // MLT
    //----------------------
    pEbrew->tmlt_sw      = false;
    pEbrew->tmlt_fx      = 0.0;
    pEbrew->vmlt_sw      = false;
    pEbrew->vmlt_fx      = 0.0;
    //----------------------
    // Boil-Kettle
    //----------------------
    pEbrew->tset_boil_sw  = false;
    pEbrew->tset_boil_fx  = 0.0;
    pEbrew->tboil_sw      = false;
    pEbrew->tboil_fx      = 0.0;
    pEbrew->vboil_sw      = false;
    pEbrew->vboil_fx      = 0.0;
    pEbrew->gamma_boil_sw = false;
    pEbrew->gamma_boil_fx = 0.0;
} // DialogEditFixParameters::on_buttonBox_rejected()

void DialogEditFixParameters::set_switch_fix(void)
{
    //----------------------
    // HLT
    //----------------------
    if (ui->cbHltSp->checkState())
    {
        pEbrew->tset_hlt_sw = true;
        pEbrew->tset_hlt_fx = ui->sbHltSp->value();
    } // if
    else
    {
        pEbrew->tset_hlt_sw = false;
        pEbrew->tset_hlt_fx = 0.0;
    } // else

    if (ui->cbHltTemp->checkState())
    {
        pEbrew->thlt_sw = true;
        pEbrew->thlt_fx = ui->sbHltTemp->value();
    } // if
    else
    {
        pEbrew->thlt_sw = false;
        pEbrew->thlt_fx = 0.0;
    } // else

    if (ui->cbHltVol->checkState())
    {
        pEbrew->vhlt_sw = true;
        pEbrew->vhlt_fx = ui->sbHltVol->value();
    } // if
    else
    {
        pEbrew->vhlt_sw = false;
        pEbrew->vhlt_fx = 0.0;
    } // else

    if (ui->cbHltPid->checkState())
    {
        pEbrew->gamma_hlt_sw = true;
        pEbrew->gamma_hlt_fx = ui->sbHltPid->value();
    } // if
    else
    {
        pEbrew->gamma_hlt_sw = false;
        pEbrew->gamma_hlt_fx = 0.0;
    } // else
    //----------------------
    // MLT
    //----------------------
    if (ui->cbMltTemp->checkState())
    {
        pEbrew->tmlt_sw = true;
        pEbrew->tmlt_fx = ui->sbMltTemp->value();
    } // if
    else
    {
        pEbrew->tmlt_sw = false;
        pEbrew->tmlt_fx = 0.0;
    } // else

    if (ui->cbMltVol->checkState())
    {
        pEbrew->vmlt_sw = true;
        pEbrew->vmlt_fx = ui->sbMltVol->value();
    } // if
    else
    {
        pEbrew->vmlt_sw = false;
        pEbrew->vmlt_fx = 0.0;
    } // else
    //----------------------
    // Boil-Kettle
    //----------------------
    if (ui->cbBkSp->checkState())
    {
        pEbrew->tset_boil_sw = true;
        pEbrew->tset_boil_fx = ui->sbBkSp->value();
    } // if
    else
    {
        pEbrew->tset_boil_sw = false;
        pEbrew->tset_boil_fx = 0.0;
    } // else

    if (ui->cbBkTemp->checkState())
    {
        pEbrew->tboil_sw = true;
        pEbrew->tboil_fx = ui->sbBkTemp->value();
    } // if
    else
    {
        pEbrew->tboil_sw = false;
        pEbrew->tboil_fx = 0.0;
    } // else

    if (ui->cbBkVol->checkState())
    {
        pEbrew->vboil_sw = true;
        pEbrew->vboil_fx = ui->sbBkVol->value();
    } // if
    else
    {
        pEbrew->vboil_sw = false;
        pEbrew->vboil_fx = 0.0;
    } // else

    if (ui->cbBkPid->checkState())
    {
        pEbrew->gamma_boil_sw = true;
        pEbrew->gamma_boil_fx = ui->sbBkPid->value();
    } // if
    else
    {
        pEbrew->gamma_boil_sw = false;
        pEbrew->gamma_boil_fx = 0.0;
    } // else
    //-------------------------
    // No sw/fx, set only once
    //-------------------------
    if (ui->cbStd->checkState())
    {   // ebrew_std
        pEbrew->ebrew_std = ui->comboStd->currentText().leftRef(2).toInt();
    } // if
    if (ui->cbSpIdx->checkState())
    {   // sp_idx
        int x = ui->sbSpIdx->value();
        if (x < pEbrew->RegEbrew->value("SP_BATCHES").toInt()) pEbrew->sp_idx = x;
    } // if
    if (ui->cbMsIdx->checkState())
    {   // ms_idx
        int x = ui->sbMsIdx->value();
        if (x < pEbrew->ms_tot) pEbrew->ms_idx = x;
    } // if
} // set_switch_fix()
