//
//  Motor.c
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
#include "Motor.h"
#include "Error.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

Motor* Motor_new(MotorGroup* group, String name, PWMPort port, MotorType type, bool reversed, 
    I2c i2c) 
{
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Motor* ret = malloc(sizeof(Motor));
    ret->type       = DeviceType_Motor;
    ret->name       = name;
    ret->parent     = group;
    ret->port       = port;
    ret->motorType  = type;
    ret->reversed   = reversed;
    ret->i2c        = i2c;
    Device_addPWM(port, (Device*) ret);
    if(i2c) {
        Device_addI2c(i2c, (Device*) ret);
    }
    return ret;
}

Motor* Motor_delete(Motor* motor) {
    if(motor) {
        SetMotor(motor->port, 0);
        Device_remove((Device*) motor);
        free(motor);
    }
    return NULL;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

PWMPort Motor_getPort(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->port;
}

MotorType Motor_getMotorType(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->motorType;
}

I2c Motor_getI2c(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->i2c;
}

MotorGroup* Motor_getGroup(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->parent;
}

bool Motor_isReversed(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->reversed;
}
