#include "dialogeditfixparameters.h"
#include "ui_dialogeditfixparameters.h"

DialogEditFixParameters::DialogEditFixParameters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditFixParameters)
{
    pEbrew = dynamic_cast<MainEbrew *>(parent);

    ui->setupUi(this);
}

DialogEditFixParameters::~DialogEditFixParameters()
{
    delete ui;
}

//---------------------------------------------------------------
// Hot Liquid Tun (HLT)
//---------------------------------------------------------------
void DialogEditFixParameters::on_cbHltSp_stateChanged(int arg1)
{
    if (arg1)
    {
        ui->sbHltSp->setEnabled(true);
        ui->sbHltSp->setFocus();
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
} // set_switch_fix()
