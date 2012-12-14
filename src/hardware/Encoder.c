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

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct Encoder {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    DigitalPort   portA;
    DigitalPort   portB;
    bool          reverse;
    bool          enabled;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Encoder* Encoder_createQuadrature(String name, DigitalPort portA, DigitalPort portB, bool reverse) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    
    Encoder* ret = malloc(sizeof(Encoder));
    ret->type    = DeviceType_QuadratureEncoder;
    ret->name    = name;
    ret->portA   = portA;
    ret->portB   = portB;
    ret->reverse = reverse;
    ret->enabled = false;
    Device_addDigital(portA, DigitalPortMode_Input, (Device*) ret);
    Device_addDigital(portB, DigitalPortMode_Input, (Device*) ret);
    return ret;
}

Encoder* Encoder_create(String name, DigitalPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Encoder* ret = malloc(sizeof(Encoder));
    ret->type    = DeviceType_Encoder;
    ret->name    = name;
    ret->portA   = port;
    ret->portB   = 0;
    ret->enabled = false;
    Device_addDigital(port, DigitalPortMode_Input, (Device*) ret);
    return ret;
}

Encoder* Encoder_delete(Encoder* encoder) {
    if(encoder) {
        Device_remove((Device*) encoder);
        free(encoder);
    }
    return NULL;
}

bool Encoder_getEnabled(Encoder* encoder) {
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

long Encoder_get(Encoder* encoder) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            return GetQuadEncoder(encoder->portA, encoder->portB);
        case DeviceType_Encoder:
            return GetEncoder(encoder->portA);
        default: 
            // will not occur //
            return 0;
    }
}

void Encoder_set(Encoder* encoder, long value) {
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);

    switch(encoder->type) {
        case DeviceType_QuadratureEncoder:
            PresetQuadEncoder(encoder->portA, encoder->portB, value);
        case DeviceType_Encoder:
            PresetEncoder(encoder->portA, value);
            break;
        default: 
            // will not occur //
            break;
    }
}

