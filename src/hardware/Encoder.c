//
//  Encoder.c
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

#define TicksPerRev_QUAD_ENCODER    360.0
#define TicksPerRev_OLD_ENCODER     100.0

struct Encoder {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    Subsystem*    subsystem;
    // device item fields //
    DigitalPort   portA;
    DigitalPort   portB;
    bool          reverse;
    bool          enabled;
    float         scale;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Encoder* Encoder_newQuadrature(String name, DigitalPort portA, DigitalPort portB, bool reverse) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    
    Encoder* ret = malloc(sizeof(Encoder));
    ret->type    = DeviceType_QuadratureEncoder;
    ret->name    = name;
    ret->portA   = portA;
    ret->portB   = portB;
    ret->reverse = reverse;
    ret->enabled = false;
    ret->scale   = (360.0 / TicksPerRev_QUAD_ENCODER);
    Device_addDigital(portA, DigitalPortMode_Input, (Device*) ret);
    Device_addDigital(portB, DigitalPortMode_Input, (Device*) ret);
    return ret;
}

Encoder* Encoder_new(String name, DigitalPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Encoder* ret = malloc(sizeof(Encoder));
    ret->type    = DeviceType_Encoder;
    ret->name    = name;
    ret->portA   = port;
    ret->portB   = 0;
    ret->reverse = false;
    ret->enabled = false;
    ret->scale   = (360.0 / TicksPerRev_OLD_ENCODER);
    Device_addDigital(port, DigitalPortMode_Input, (Device*) ret);
    return ret;
}

DigitalPort Encoder_getPort(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    
    return encoder->portA;
}

DigitalPort Encoder_getPort2(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    
    return encoder->portB;
}

bool Encoder_isReversed(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    return encoder->reverse;
}

bool Encoder_isEnabled(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    return encoder->enabled;
}

void Encoder_setEnabled(Encoder* encoder, bool value) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            if(value) {
                StartQuadEncoder(encoder->portA, encoder->portB, encoder->reverse);
            } else {
                StopQuadEncoder(encoder->portA, encoder->portB);
            }
            break;
        case DeviceType_Encoder:
            if(value) {
                StartEncoder(encoder->portA);
            } else {
                StopEncoder(encoder->portA);
            }
            break;
        default: 
            // will not occur //
            return;
    }
    encoder->enabled = value;
}

float Encoder_getScaleFactor(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    return encoder->scale;
}

void Encoder_setScaleFactor(Encoder* encoder, float scale) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    ErrorIf(scale == 0.0, VEXOS_ARGINVALID);

    encoder->scale = scale;
}

long Encoder_getRaw(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            return GetQuadEncoder(encoder->portA, encoder->portB);
        case DeviceType_Encoder:
            return GetEncoder(encoder->portA);
        default: 
            return 0;
    }
}

void Encoder_presetRaw(Encoder* encoder, long value) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    
    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            PresetQuadEncoder(encoder->portA, encoder->portB, value);
            break;
        case DeviceType_Encoder:
            PresetEncoder(encoder->portA, value);
            break;
        default: 
            break;
    }
}


float Encoder_get(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            return GetQuadEncoder(encoder->portA, encoder->portB) * encoder->scale;
        case DeviceType_Encoder:
            return GetEncoder(encoder->portA) * encoder->scale;
        default: 
            return 0.0;
    }
}

void Encoder_preset(Encoder* encoder, float value) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    
    long ivalue = value / encoder->scale;
    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            PresetQuadEncoder(encoder->portA, encoder->portB, ivalue);
            break;
        case DeviceType_Encoder:
            PresetEncoder(encoder->portA, ivalue);
            break;
        default: 
            break;
    }
}

