//
//  Accelerometer.c
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

struct Accelerometer {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    AnalogPort    port;
    bool          enabled;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Accelerometer* Accelerometer_new(String name, AnalogPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Accelerometer* ret = malloc(sizeof(Accelerometer));
    ret->type     = DeviceType_Accelerometer;
    ret->name     = name;
    ret->port     = port;
    ret->enabled  = false;
    Device_addAnalog(port, (Device*) ret);
    return ret;
}

Accelerometer* Accelerometer_delete(Accelerometer* accel) {
    if(accel) {
        Device_remove((Device*) accel);
        free(accel);
    }
    return NULL;
}

AnalogPort Accelerometer_getPort(Accelerometer* accel) {
    ErrorIf(accel == NULL, VEXOS_ARGNULL);
    
    return accel->port;
}

void Accelerometer_init(Accelerometer* accel) {
    ErrorIf(accel == NULL, VEXOS_ARGNULL);

    InitAccelerometer(accel->port);
}

bool Accelerometer_isEnabled(Accelerometer* accel) {
    ErrorIf(accel == NULL, VEXOS_ARGNULL);

    return accel->enabled;
}

void Accelerometer_setEnabled(Accelerometer* accel, bool value) {
    ErrorIf(accel == NULL, VEXOS_ARGNULL);

    if(value) {
        StartAccelerometer(accel->port);
    } else {
        StopAccelerometer(accel->port);
    }
    accel->enabled = value;
}

float Accelerometer_getAccelerationG(Accelerometer* accel) {
    ErrorIf(accel == NULL, VEXOS_ARGNULL);

    return GetGyroAngle(accel->port) / 1000.0;
}

