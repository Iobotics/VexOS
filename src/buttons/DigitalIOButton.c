//
//  DigitalIOButton.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/06/2012.
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

#include "ButtonClass.h"
#include "Hardware.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareButtonClass(DigitalIOButton, {
    DigitalPort port;
});

static void constructor(va_list argp) {
    self->fields->port = va_arg(argp, int);
    setArgs("%d", self->fields->port);
}

static bool get() {
    return !GetDigitalInput(self->fields->port);
}

const ButtonClass DigitialIOButton = {
    .name        = "DigitialIOButton",
    .constructor = &constructor,
    .get         = &get
};
