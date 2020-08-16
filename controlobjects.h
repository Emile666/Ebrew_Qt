/**************************************************************************************
** Filename    : controlobjects.h
** Author      : Emile
** Purpose     : Header file for controlobjects.cpp. Contains various control objects,
**               such as the moving-average filter, the PID-controller and the
**               slope-limiter.
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
#ifndef CONTROL_OBJECTS_H
#define CONTROL_OBJECTS_H
#include <QObject>

#define MA_MAX_N (20) /* Max. order N for moving_average filter */
#define PID_OFF   (0) /* PID-controller is OFF */
#define PID_ON    (1) /* PID-controller is ON */
#define PID_FFC   (2) /* PID-controller in feed-forward mode */

//------------------------------------------------------------------------------------------
// PID-controller
//------------------------------------------------------------------------------------------
class PidCtrl : public QObject
{
    Q_OBJECT

public:
    PidCtrl(qreal Kc, qreal Ti, qreal Td, qreal Ts);
    qreal   pidControl(qreal xk, qreal tset);
    void    pidInit(qreal Kc, qreal Ti, qreal Td, qreal Ts);
    void    pidEnable(uint8_t enable);
    void    pidSetLimits(qreal min, qreal max);
    uint8_t pidGetStatus(void);

protected:
    uint8_t pid_on; // true = pid-controller is enabled
    qreal   xk_2;   // x[k-2], previous value of x[k-1] (= measured temperature)
    qreal   xk_1;   // x[k-1], previous value of the input variable x[k] (= measured temperature)
    qreal   yk;     // y[k], pid-controller output (%)
    qreal   kp;     // = Kc
    qreal   ki;     // = Kc*Ts/Ti
    qreal   kd;     // = Kc*Ti/Td
    qreal   pp;     // output of P-action
    qreal   pi;     // output of I-action
    qreal   pd;     // output of D-action
    qreal   ykmin;  // Minimum value for output yk
    qreal   ykmax;  // Maximum value for output yk
}; // class PidCtrl

//------------------------------------------------------------------------------------------
// Moving-average filter
//------------------------------------------------------------------------------------------
class MA : public QObject
{
    Q_OBJECT

public:
    MA(uint8_t order, qreal init_val);
    void  ma_init(uint8_t order, qreal init_val);
    qreal moving_average(qreal x);
    qreal   T[MA_MAX_N]; // array with delayed values of input signal
    uint8_t index;       // index in T[] where to store the new input value
    qreal   sum;         // The running sum of the MA filter

protected:
    uint8_t N;           // order of MA-filter, N < MA_MAX_N
}; // clas MA

//------------------------------------------------------------------------------------------
// Slope-limiter
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
