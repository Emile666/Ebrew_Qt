#ifndef DIALOGVIEWPROGRESS_H
#define DIALOGVIEWPROGRESS_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogViewProgress;
}

static const QString   MashTitle("  Mash Temp Time  PreHt Timer Timer        Time-\n"
                                 "  idx  [°C] [sec] [sec] [sec] Status       Stamp\n"
                                 "------------------------------------------------");
static const QString SpargeTitle("  index  From HLT -> MLT  |   From MLT -> Boil-kettle\n"
                                 "--------------------------|--------------------------");

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
    int        prev_ebrew_std = S00_INITIALISATION; // previous value of ebrew_std

    /* Time-stamps for Sparge, Boil and Chilling*/
    QStringList mlt2boil; // strings for time-stamp moment of MLT -> BOIL
    QStringList hlt2mlt;  // MAX_SP strings for time-stamp moment of HLT -> MLT
    QStringList Boil;     // Boil-start and Boil-End time-stamps
    QStringList Chill;    // Chill-start and Chill-End time-stamps
};

#endif // DIALOGVIEWPROGRESS_H
