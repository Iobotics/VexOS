//
//  UniLiftHome.c
//  VexOS for Vex Cortex, Universal Robot Library
//
//  Created by Jeff Malins on 12/29/2012.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
//
//  --------------------------------------------------------------------------
//
//  Some algorithms, structures and code organization used by this file have
//  been adapted from the WPILibJ software package. Those portions are:
//
//  Copyright (c) FIRST 2008-2012. All rights reserved.
//  
//  WPILibJ elements are distributed in this program in accordance to their 
//  original FIRST BSD License, available as 'BSD_License_for_WPILib_code.txt' 
//  present in this distribution.
//

#include "CommandClass.h"
#include "UniBot.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineCommandClass(UniLiftHome, {
    int   homePosition;
    Power power;
});

static void constructor(va_list argp) {
    self->fields->homePosition = va_arg(argp, int);
    self->fields->power        = (float) va_arg(argp, double);
    String name = UniLift_getPositionName(self->fields->homePosition);
    ErrorMsgIf(!name, VEXOS_ARGINVALID, "Invalid lift position: %d", self->fields->homePosition);
    ErrorMsgIf(!UniLift_hasHomeSwitch(), VEXOS_OPINVALID, "Lift does not have a home switch");
    setArgs("%s, %f", name, self->fields->power);
    require(&UniLift);
}

static void initialize() {
    UniLift_setPower(self->fields->power);
}

static void execute() { }

static bool isFinished() {
    return UniLift_getHomeSwitch();
}

static void end() { 
    UniLift_setPower(0);
    UniLift_resetPosition(self->fields->homePosition);
}

static void interrupted() { 
    UniLift_setPower(0);
}

