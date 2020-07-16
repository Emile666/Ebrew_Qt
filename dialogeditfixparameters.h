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
