//
//  DigitalOut.c
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/13/2012.
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

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct DigitalOut {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    DigitalPort   port;
    bool          value;
};

static DigitalOut* new(String name, DeviceType type, DigitalPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    DigitalOut* ret = malloc(sizeof(DigitalOut));
    ret->type  = type;
    ret->name  = name;
    ret->value = false;
    Device_addDigital(port, DigitalPortMode_Output, (Device*) ret);
    return ret;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

DigitalOut* DigitalOut_createPneumaticValve(String name, DigitalPort port) {
    ErrorEntryPoint();
    DigitalOut* out = new(name, DeviceType_PneumaticValve, port);
    ErrorEntryClear();
    return out;
}

DigitalOut* DigitalOut_createLED(String name, DigitalPort port) {
    ErrorEntryPoint();
    DigitalOut* out = new(name, DeviceType_LED, port);
    ErrorEntryClear();
    return out;
}

DigitalOut* DigitalOut_delete(DigitalOut* out) {
    if(out) {
        Device_remove((Device*) out);
        free(out);
    }
    return NULL;
}

DigitalPort DigitalOut_getPort(DigitalOut* out) {
    ErrorIf(out == NULL, VEXOS_ARGNULL);
    
    return out->port;
}

bool DigitalOut_get(DigitalOut* out) {
    ErrorIf(out == NULL, VEXOS_ARGNULL);

    return out->value;
}

void DigitalOut_set(DigitalOut* out, bool value) {
    ErrorIf(out == NULL, VEXOS_ARGNULL);

    SetDigitalOutput(out->port, value);
    out->value = value;
}
