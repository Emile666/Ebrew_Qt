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
#ifndef MAIN_EBREW_H
#define MAIN_EBREW_H

#include <QObject>
#include "hmi_objects.h"

//------------------------------------------------------------------------------------------
class MainEbrew : public QObject
{
public:
    MainEbrew(void);

protected:
	tank *hlt;  // Pointer to HLT object
	tank *mlt;  // Pointer to MLT object
	tank *boil; // Pointer to Boil-kettle object
	valve *V1;  // Pointer to valve V1
	valve *V2;  // Pointer to valve V2
	valve *V3;  // Pointer to valve V3
	valve *V4;  // Pointer to valve V5
	valve *V6;  // Pointer to valve V6
	valve *V7;  // Pointer to valve V7
	pump  *P1;  // Pointer to pump P1
	pump  *P2;  // Pointer to pump P2;

	qreal thlt;             // HLT actual temperature
	qreal thlt_offset;      // Offset to add to Thlt measurement
	qreal tmlt;             // MLT actual temperature
	qreal tmlt_offset;      // Offset to add to Tmlt measurement
	qreal tboil;            // Boil-kettle actual temperature
	qreal tboil_offset;     // Offset to add to Tboil measurement
	qreal tcfc;             // CFC-output actual temperature
	qreal tcfc_offset;      // Offset to add to Tcfc measurement
	qreal tset_slope_limit; // Slope limiter for Temp. Setpoints
	qreal gamma_hlt;        // PID controller output for HLT
	qreal gamma_boil;       // PID controller output for Boil-Kettle
	qreal tset_hlt;         // HLT reference temperature
	qreal tset_mlt;         // MLT reference temperature
	qreal tset_boil;        // HLT reference temperature
}; // MainEbrew()

//------------------------------------------------------------------------------------------

#endif // MAIN_EBREW_H
