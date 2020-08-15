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
