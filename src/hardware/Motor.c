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

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define MAX_MOTOR_POWER     127

struct Motor {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    PWMPort       port;
    MotorType     motorType;
    I2c           i2c;
    MotorGroup*   parent;
    Power         power;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Motor_setParent(Motor* motor, MotorGroup* group) {
    motor->parent = group;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Motor* Motor_new(String name, PWMPort port, MotorType type) {
    ErrorEntryPoint();
    Motor* motor = Motor_newWithIME(name, port, type, 0);
    ErrorEntryClear();
    return motor;
}

Motor* Motor_newWithIME(String name, PWMPort port, MotorType type, I2c i2c) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Motor* ret = malloc(sizeof(Motor));
    ret->type       = DeviceType_Motor;
    ret->name       = name;
    ret->port       = port;
    ret->motorType  = type;
    ret->i2c        = i2c;
    ret->power      = 0.0;
    Device_addPWM(port, (Device*) ret);
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

Power Motor_get(Motor* motor) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);

    return motor->power;
}

void Motor_set(Motor* motor, Power power) {
    ErrorIf(motor == NULL, VEXOS_ARGNULL);
    ErrorIf(power < -1.0 || power > 1.0, VEXOS_ARGRANGE);
    if(power == motor->power) return;

    SetMotor(motor->port, (int) (power * MAX_MOTOR_POWER));
    motor->power = power;
}
