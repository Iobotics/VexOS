//
//  AnalogIn.c
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

#include "API.h"

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define POTENTIOMETER_TRAVEL_DEGREES    250

struct AnalogIn {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    AnalogPort    port;
    float         scale;
    int           offset;
};

static AnalogIn* new(String name, DeviceType type, AnalogPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    AnalogIn* ret = malloc(sizeof(AnalogIn));
    ret->type   = type;
    ret->name   = name;
    ret->port   = port;
    ret->scale  = 1.0;
    ret->offset = 0;
    Device_addAnalog(port, (Device*) ret);
    return ret;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

AnalogIn* AnalogIn_newPotentiometer(String name, AnalogPort port) {
    ErrorEntryPoint();
    AnalogIn* in = new(name, DeviceType_Potentiometer, port);
    in->scale = POTENTIOMETER_TRAVEL_DEGREES / (360.0 * MAX_ANALOG_IN_RAW);
    ErrorEntryClear();
    return in;
}

AnalogIn* AnalogIn_newLineFollower(String name, AnalogPort port) {
    ErrorEntryPoint();
    AnalogIn* in = new(name, DeviceType_LineFollower, port);
    ErrorEntryClear();
    return in;
}

AnalogIn* AnalogIn_newLightSensor(String name, AnalogPort port) {
    ErrorEntryPoint();
    AnalogIn* in = new(name, DeviceType_LightSensor, port);
    ErrorEntryClear();
    return in;
}

AnalogIn* AnalogIn_delete(AnalogIn* in) {
    if(in) {
        Device_remove((Device*) in);
        free(in);
    }
    return NULL;
}

AnalogPort AnalogIn_getPort(AnalogIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);
    
    return in->port;
}

float AnalogIn_getScaleFactor(AnalogIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);
    
    return in->scale;
}

void AnalogIn_setScaleFactor(AnalogIn* in, float scale) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);
    ErrorIf(scale == 0.0, VEXOS_ARGINVALID);
    
    in->scale = scale;
}

int AnalogIn_getRaw(AnalogIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);
    
    return GetAnalogInputHR(in->port);
}

void AnalogIn_presetRaw(AnalogIn* in, int value) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    in->offset = GetAnalogInputHR(in->port) - value;
}

float AnalogIn_get(AnalogIn* in) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    return (GetAnalogInputHR(in->port) - in->offset) * in->scale;
}

void AnalogIn_preset(AnalogIn* in, float value) {
    ErrorIf(in == NULL, VEXOS_ARGNULL);

    in->offset = GetAnalogInputHR(in->port) - (int)(value / in->scale);
}

