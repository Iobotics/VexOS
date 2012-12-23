//
//  DigitalIn.c
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/12/2012.
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

#include "API.h"

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct DigitalIn {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    DigitalPort   port;
    Button*       button;
    bool          inverted;
    InterruptMode interruptMode;
};

static DigitalIn* new(String name, DeviceType type, DigitalPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    DigitalIn* ret = malloc(sizeof(DigitalIn));
    ret->name          = name;
    ret->type          = type;
    ret->port          = port;
    ret->button        = NULL;
    ret->inverted      = false;
    ret->interruptMode = InterruptMode_Disabled;
    Device_addDigital(port, DigitalPortMode_Input, (Device*) ret);
    return ret;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

DigitalIn* DigitalIn_newBump(String name, DigitalPort port) {
    ErrorEntryPoint();
    DigitalIn* in = new(name, DeviceType_BumpSwitch, port);
    in->inverted  = true;
    ErrorEntryClear();
    return in;
}

DigitalIn* DigitalIn_newLimit(String name, DigitalPort port) {
    ErrorEntryPoint();
    DigitalIn* in = new(name, DeviceType_LimitSwitch, port);
    in->inverted  = true;
    ErrorEntryClear();
    return in;
}

DigitalIn* DigitalIn_newJumper(String name, DigitalPort port) {
    ErrorEntryPoint();
    DigitalIn* in = new(name, DeviceType_Jumper, port);
    in->inverted  = true;
    ErrorEntryClear();
    return in;
}

DigitalPort DigitalIn_getPort(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    return in->port;
}

bool DigitalIn_isInverted(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    return in->inverted;
}

void DigitalIn_setInverted(DigitalIn* in, bool value) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    InterruptMode imode = in->interruptMode;
    if(imode != InterruptMode_Disabled) {
        DigitalIn_setInterruptMode(in, InterruptMode_Disabled);
    }
    in->inverted = value;
    if(imode != InterruptMode_Disabled) {
        DigitalIn_setInterruptMode(in, imode);
    }
}

bool DigitalIn_get(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    return GetDigitalInput(in->port) ^ in->inverted;
}

InterruptMode DigitalIn_getInterruptMode(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    return in->interruptMode;
}

void DigitalIn_setInterruptMode(DigitalIn* in, InterruptMode mode) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);
    ErrorIf(mode < InterruptMode_Disabled || mode > InterruptMode_FallingEdge, VEXOS_ARGRANGE);

    in->interruptMode = mode;
    switch(mode) {
        case InterruptMode_Disabled:
            if(in->interruptMode != InterruptMode_Disabled) {
                StopInterruptWatcher(in->port);
            }
            break;
        case InterruptMode_RisingEdge:
        case InterruptMode_FallingEdge:
            if(in->inverted) {
                mode = (mode == InterruptMode_RisingEdge)? 
                            InterruptMode_FallingEdge: 
                            InterruptMode_RisingEdge;
            }
            StartInterruptWatcher(in->port, mode);
            break;
    }
}

bool DigitalIn_getInterrupted(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    if(!in->interruptMode == InterruptMode_Disabled) return false;
    return GetInterruptWatcher(in->port);
}

Button* DigitalIn_getButton(DigitalIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    if(in->button) return in->button;
    return (in->button = Button_new(&DigitalIOButton, in->port));
}
