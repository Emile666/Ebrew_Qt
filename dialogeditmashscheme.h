/**************************************************************************************
** Filename    : dialogeditmashscheme.h
** Author      : Emile
** Purpose     : Header file for dialogeditmashscheme.cpp. Contains a dialog screen
**               with which to read-from-file, change and set a mash scheme.
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
#ifndef DIALOGEDITMASHSCHEME_H
#define DIALOGEDITMASHSCHEME_H

#include <QDialog>
#include "MainEbrew.h"

namespace Ui {
class DialogEditMashScheme;
}

class DialogEditMashScheme : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditMashScheme(QWidget *parent = nullptr);
    ~DialogEditMashScheme();

private slots:
    void on_pushButton_clicked();   // Load Mash Scheme File...
    void on_pushButton_2_clicked(); // Save As...
    void on_pushButton_3_clicked(); // Save

private:
    Ui::DialogEditMashScheme *ui;
    MainEbrew *pEbrew; // pointer to parent (MainEbrew)
};

#endif // DIALOGEDITMASHSCHEME_H
