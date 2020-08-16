/**************************************************************************************
** Filename    : draw_hmi_screen.h
** Author      : Emile
** Purpose     : Header file for draw_hmi_screen.cpp. Contains a single procedure that
**               draws all graphical objects to the screen.
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
#ifndef DRAW_HMI_SCREEN_H
#define DRAW_HMI_SCREEN_H

#include <QGraphicsScene>

#include "hmi_objects.h"
#include "MainEbrew.h"

//------------------------------------------------------------------------------------------
void draw_hmi_screen(QGraphicsScene *scene, MainEbrew *p);

//------------------------------------------------------------------------------------------

#endif // DRAW_HMI_SCREEN
