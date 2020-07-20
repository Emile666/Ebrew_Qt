/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef CONTROL_OBJECTS_H
#define CONTROL_OBJECTS_H
#include <QObject>

#define MA_MAX_N (20) /* Max. order N for moving_average filter */

//------------------------------------------------------------------------------------------
class PidCtrl : public QObject
{
    Q_OBJECT

public:
    PidCtrl(qreal Kc, qreal Ti, qreal Td, qreal Ts);
    qreal pid_control(qreal xk, qreal tset);
    void  pid_init(qreal Kc, qreal Ti, qreal Td, qreal Ts);
    void  pid_enable(bool enable);

protected:
    bool  pid_on; // true = pid-controller is enabled
    qreal xk_2;   // x[k-2], previous value of x[k-1] (= measured temperature)
    qreal xk_1;   // x[k-1], previous value of the input variable x[k] (= measured temperature)
    qreal yk;     // y[k], pid-controller output (%)
    qreal kp;     // = Kc
    qreal ki;     // = Kc*Ts/Ti
    qreal kd;     // = Kc*Ti/Td
    qreal pp;     // output of P-action
    qreal pi;     // output of I-action
    qreal pd;     // output of D-action
}; // class PidCtrl

//------------------------------------------------------------------------------------------
class MA : public QObject
{
    Q_OBJECT

public:
    MA(uint8_t order, qreal init_val);
    void  ma_init(uint8_t order, qreal init_val);
    qreal moving_average(qreal x);

protected:
    uint8_t N;           // order of MA-filter, N < MA_MAX_N
    qreal   T[MA_MAX_N]; // array with delayed values of input signal
    uint8_t index;       // index in T[] where to store the new input value
    qreal   sum;         // The running sum of the MA filter
}; // clas MA

//------------------------------------------------------------------------------------------
class SlopeLimiter : public QObject
{
    Q_OBJECT

public:
    SlopeLimiter(void);
    SlopeLimiter(const qreal lim);
    void  setLim(const qreal lim);
    qreal slopeLimit(qreal value);
private:
    qreal prev_val;
    qreal slope_limit;
}; // class SlopeLimiter

#endif // CONTROL_OBJECTS_H
