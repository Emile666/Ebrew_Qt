/**************************************************************************************
** Filename    : dialogeditterminal.h
** Author      : Emile
** Purpose     : Header file for dialogeditterminal.cpp. Contains a terminal editor
**               with which to send commands to the Ebrew hardware directly.
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
#ifndef DIALOGEDITTERMINAL_H
#define DIALOGEDITTERMINAL_H

#include <QDialog>
#include <QString>
#include "MainEbrew.h"

namespace Ui {
class DialogEditTerminal;
}

class DialogEditTerminal : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditTerminal(QWidget *parent = nullptr);
    ~DialogEditTerminal();

public slots:
    void readSerialPort(void);

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void on_buttonBox_clicked(QAbstractButton *);

private:
    Ui::DialogEditTerminal *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
    QByteArray cmdString;
};

#endif // DIALOGEDITTERMINAL_H
