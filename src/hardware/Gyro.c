//
//  Gyro.c
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

#define GYRO_TYPE 16 // LY3100ALH //

struct Gyro {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    AnalogPort    port;
    bool          enabled;
    int           deadband;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Gyro* Gyro_new(String name, AnalogPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Gyro* ret = malloc(sizeof(Gyro));
    ret->type     = DeviceType_Gyro;
    ret->name     = name;
    ret->port     = port;
    ret->enabled  = false;
    ret->deadband = GYRO_DEFAULT_DEADBAND;
    Device_addAnalog(port, (Device*) ret);
    return ret;
}

AnalogPort Gyro_getPort(Gyro* gyro) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);
    
    return gyro->port;
}

void Gyro_init(Gyro* gyro) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);

    SetGyroType(gyro->port, GYRO_TYPE);
    InitGyro(gyro->port);
}

bool Gyro_isEnabled(Gyro* gyro) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);

    return gyro->enabled;
}

void Gyro_setEnabled(Gyro* gyro, bool value) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);

    if(value) {
        StartGyro(gyro->port);
    } else {
        StopGyro(gyro->port);
    }
    gyro->enabled = value;
}

int Gyro_getDeadband(Gyro* gyro) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);

    return gyro->deadband;
}

void Gyro_setDeadband(Gyro* gyro, int value) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);
    ErrorIf(value < 0, VEXOS_ARGRANGE);

    SetGyroDeadband(gyro->port, value);
    gyro->deadband = value;
}

float Gyro_getAngleDegrees(Gyro* gyro) {
    ErrorIf(gyro == NULL, VEXOS_ARGNULL);

    return GetGyroAngle(gyro->port) / 10.0;
}
