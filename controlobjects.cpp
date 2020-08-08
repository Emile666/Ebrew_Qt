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

#include "controlobjects.h"

//------------------------------------------------------------------------------------------
PidCtrl::PidCtrl(qreal Kc, qreal Ti, qreal Td, qreal Ts)
{
    pidInit(Kc, Ti, Td, Ts); // init. pid-controller
    pidEnable(PID_OFF);      // disable pid-controller at start-up
    pidSetLimits(0.0,100.0); // output between 0% and 100%
} // PidCtrl::PidCtrl()

/*------------------------------------------------------------------
  Purpose  : This function initialises the PID controller.
  Variables: Kc: Proportional value set by user [%/C]
             Ti: integral time-constant [sec.]
             Td: differential time-constant [sec.]
             Ts: sample-time period [sec.]

                   Kc.Ts
             ki =  -----   (for I-term)
                    Ti

                       Td
             kd = Kc . --  (for D-term)
                       Ts

  Returns  : No values are returned
  ------------------------------------------------------------------*/
void PidCtrl::pidInit(qreal Kc, qreal Ti, qreal Td, qreal Ts)
{
    kp = Kc;
    if (Ti < 1e-3)
         ki = 0.0;
    else ki = Kc * Ts / Ti;
    if (Ts < 1e-3)
         kd = 0.0;
    else kd = Kc * Td / Ts;
} // PidCtrl::pid_init()

/*------------------------------------------------------------------
  Purpose  : This function implements the Takahashi Type C PID
             controller: the P and D term are no longer dependent
             on the set-point, only on the setpoint (which is tset).
             This function should be called once every TS seconds.
  Variables:
        xk : The input variable x[k] (= measured temperature)
        yk : The output variable y[k] (= percentage for power electronics)
      tset : The setpoint value for the temperature
  Returns  : y[k], pid-controller output
  ------------------------------------------------------------------*/
qreal PidCtrl::pidControl(qreal xk, qreal tset)
{
    if (pid_on == PID_ON)
    {
       //--------------------------------------------------------------------------------
       // Takahashi Type C PID controller:
       //
       //                                    Kc.Ts        Kc.Td
       // y[k] = y[k-1] + Kc.(x[k-1]-x[k]) + -----.e[k] + -----.(2.x[k-1]-x[k]-x[k-2])
       //                                      Ti           Ts
       //
       //--------------------------------------------------------------------------------
       pp = kp * (xk_1 - xk);              // Kc.(x[k-1]-x[k])
       pi = ki * (tset - xk);              // (Kc.Ts/Ti).e[k]
       pd = kd * (2.0 * xk_1 - xk - xk_2); // (Kc.Td/Ts).(2.x[k-1]-x[k]-x[k-2])
       yk  += pp + pi + pd;                // add y[k-1] + P, I & D actions to y[k]
    } // if
    else if (pid_on == PID_FFC)
    {   // Feed-forward mode
        if (tset > xk) yk = 100.0;
        else           yk =   0.0;
    } // else pid_on == PID_OFF
    else { yk = pp = pi = pd = 0.0; }

    xk_2  = xk_1; // x[k-2] = x[k-1]
    xk_1  = xk;   // x[k-1] = x[k]

    // limit y[k] to 0 % and 100 %
    if (yk > ykmax)
    {
       yk = ykmax;
    }
    else if (yk < ykmin)
    {
       yk = ykmin;
    } // else
    return yk;
} // PidCtrl::pid_control()

void PidCtrl::pidEnable(uint8_t enable)
{
    pid_on = enable; // PID_OFF, PID_ON, PID_FFC
} // PidCtrl::pid_run()

void PidCtrl::pidSetLimits(qreal min, qreal max)
{
    if (max > min)
    {
        ykmax = max;
        ykmin = min;
    }
    else
    {
        ykmax = 100.0;
        ykmin =   0.0;
    } // else
} // PidCtrl::pid_run()

uint8_t PidCtrl::pidGetStatus(void)
{
    return pid_on; // PID_OFF, PID_ON, PID_FFC
} // pidGetStatus()

//------------------------------------------------------------------------------------------
MA::MA(uint8_t N, qreal init_val)
{
    ma_init(N, init_val); // init. moving-average filter
} // PidCtrl::PidCtrl()

void MA::ma_init(uint8_t order, qreal init_val)
{
    if (order < MA_MAX_N)
         N = order;    // order of MA filter
    else N = MA_MAX_N; // limit max. order
    index = 0;         // index in cyclic array
    sum   = init_val;  // running sum
    for (int i = 0; i < N; i++)
    {
       T[i] = init_val / N; // set cyclic array to init. value
    } // for
} // MA::ma_init()

qreal MA::moving_average(qreal x)
{
    sum -= T[index];  // subtract value to overwrite from running sum
    T[index] = x / N; // store new value in array
    sum += T[index];  // update running sum with new value
    if (++index >= N) // update index in cyclic array
    {
       index = 0;     // restore to 1st position
    } // if
    return sum;       // return value = filter output
} // MA::moving_average()

//------------------------------------------------------------------------------------------
SlopeLimiter::SlopeLimiter(const double lim)
{
    prev_val = 0.0;
    setLim(lim);
} // constructor SlopeLimiter

SlopeLimiter::SlopeLimiter(void)
{
    prev_val = 0.0;
} // constructor SlopeLimiter

void SlopeLimiter::setLim(const double lim)
{
    slope_limit = lim;
} // SlopeLimiter::setLim()

/*------------------------------------------------------------------
  Purpose  : This function limits the increase of val by lim.

                              rval
                               |  ------------  lim
                               |/
              -----------------/------------------ -> val - prev_val
                              /|
              -lim ----------  |

  Variables:
       lim : The limiting value
  prev_val : The previous value of the variable to limit
       val : The output value of the variable to limit
  Returns  : rval, the new slope-limited value
  ------------------------------------------------------------------*/
qreal SlopeLimiter::slopeLimit(qreal value)
{
    qreal rval = value;

    if      (value - prev_val >  slope_limit) rval =  prev_val + slope_limit;
    else if (value - prev_val < -slope_limit) rval =  prev_val - slope_limit;
    // else: do nothing, Tnew is within slope limits
    prev_val = rval;
    return rval;
} // SlopeLimiter::slopeLimit()
