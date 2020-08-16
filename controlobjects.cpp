/**************************************************************************************
** Filename    : controlobjects.cpp
** Author      : Emile
** Purpose     : This file contains various control objects, such as the
**               moving-average filter, the PID-controller and the slope-limiter.
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
#include "controlobjects.h"

//------------------------------------------------------------------------------------------
// PID-controller
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
// Moving-average filter
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
// Slope-limiter
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
