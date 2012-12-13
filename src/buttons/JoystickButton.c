//
//  JoystickButton.c
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

#include "ButtonClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareButtonClass(JoystickButton, {
    unsigned char       joystick;
    JoystickChannelType channel;
    JoystickButtonType  button;
});

static void constructor(va_list argp) {
    self->fields->joystick = va_arg(argp, unsigned int);
    self->fields->channel  = va_arg(argp, int);
    self->fields->button   = va_arg(argp, int);
    setArgs("%d, %d, %d", self->fields->joystick, self->fields->channel, self->fields->button);
}

static bool get() {
    Fields* fields = self->fields;
    return GetJoystickDigital(fields->joystick, fields->channel, fields->button);
}
