//
//  Servo.c
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

#define SERVO_DEFAULT_PWM 127

struct Servo {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    Subsystem*    subsystem;
    // device item fields //
    PWMPort       port;
    int           pwmValue;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Servo* Servo_new(String name, PWMPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Servo* ret = malloc(sizeof(Servo));
    ret->type      = DeviceType_Servo;
    ret->name      = name;
    ret->port      = port;
    ret->pwmValue  = SERVO_DEFAULT_PWM;
    Device_addPWM(port, (Device*) ret);
    return ret;
}

PWMPort Servo_getPort(Servo* servo) {
    ErrorIf(servo == NULL, VEXOS_ARGNULL);
    
    return servo->port;
}

float Servo_getAngleDegrees(Servo* servo) {
    ErrorIf(servo == NULL, VEXOS_ARGNULL);

    return (servo->pwmValue * SERVO_TRAVEL_DEGREES) / 127.0;
}

void Servo_setAngleDegrees(Servo* servo, float angle) {
    ErrorIf(servo == NULL, VEXOS_ARGNULL);

    int value = (int) (angle * 127.0) / SERVO_TRAVEL_DEGREES;
    SetServo(servo->port, value);
    servo->pwmValue = value;
}

