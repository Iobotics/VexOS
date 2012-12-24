//
//  Joystick.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/21/2012.
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

#include "VexOS.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define JoystickChannelType_RightX  1
#define JoystickChannelType_RightY  2
#define JoystickChannelType_LeftY   3
#define JoystickChannelType_LeftX   4

#define JOYSTICK_ACCEL_X    1
#define JOYSTICK_ACCEL_Y    2

#define MAX_JOYSTICK_POWER  127.0
#define MAX_JOYSTICK_ACCEL  1024.0

struct Joystick {
    unsigned char id;
};

typedef struct {
    unsigned char channel;
    unsigned char button;
} JoystickButtonData;

static JoystickButtonData Joystick_findButton(JoystickHand hand, JoystickButtonType type) {
    JoystickButtonData data;
    switch(type) {
        case JoystickButtonType_Bottom:
        case JoystickButtonType_Top:
            data.channel = hand + 5;
            data.button  = type - 4;
            break;
        default:
            data.channel = hand + 7;
            data.button  = type;
            break;
    }
    return data;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Joystick* Joystick_new(unsigned char id) {
    ErrorIf(id < 1 || id > 2, VEXOS_ARGRANGE);
    Joystick* ret = malloc(sizeof(Joystick));
    ret->id = id;
    return ret;
}

Joystick* Joystick_delete(Joystick* stick) {
    if(stick) {
        free(stick);
    }
    return NULL;
}

unsigned char Joystick_getId(Joystick* stick) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    return stick->id;
}

float Joystick_getX(Joystick* stick, JoystickHand hand) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    if(hand == JoystickHand_Left) {
        return GetJoystickAnalog(stick->id, JoystickChannelType_LeftX) / MAX_JOYSTICK_POWER;
    } else {
        return GetJoystickAnalog(stick->id, JoystickChannelType_RightX) / MAX_JOYSTICK_POWER;
    }
}

float Joystick_getY(Joystick* stick, JoystickHand hand) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    if(hand == JoystickHand_Left) {
        return GetJoystickAnalog(stick->id, JoystickChannelType_LeftY) / MAX_JOYSTICK_POWER;
    } else {
        return GetJoystickAnalog(stick->id, JoystickChannelType_RightY) / MAX_JOYSTICK_POWER;
    }
}

void Joystick_setXDeadband(Joystick* stick, JoystickHand hand, float lower, float upper) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);
    ErrorIf(lower < -1.0 || lower > 0, VEXOS_ARGRANGE);
    ErrorIf(upper > 1.0 || upper < 0, VEXOS_ARGRANGE);

    if(hand == JoystickHand_Left) {
        SetJoystickAnalogDeadband(stick->id, JoystickChannelType_LeftX, 
                lower * MAX_JOYSTICK_POWER, upper * MAX_JOYSTICK_POWER);
    } else {
        SetJoystickAnalogDeadband(stick->id, JoystickChannelType_RightX, 
                lower * MAX_JOYSTICK_POWER, upper * MAX_JOYSTICK_POWER);
    }
}

void Joystick_setYDeadband(Joystick* stick, JoystickHand hand, float lower, float upper) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);
    ErrorIf(lower < -1.0 || lower > 0, VEXOS_ARGRANGE);
    ErrorIf(upper > 1.0 || upper < 0, VEXOS_ARGRANGE);

    if(hand == JoystickHand_Left) {
        SetJoystickAnalogDeadband(stick->id, JoystickChannelType_LeftY, 
                lower * MAX_JOYSTICK_POWER, upper * MAX_JOYSTICK_POWER);
    } else {
        SetJoystickAnalogDeadband(stick->id, JoystickChannelType_RightY, 
                lower * MAX_JOYSTICK_POWER, upper * MAX_JOYSTICK_POWER);
    }
}

Button* Joystick_getButton(Joystick* stick, JoystickHand hand, JoystickButtonType type) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);
    ErrorIf(hand < JoystickHand_Left || hand > JoystickHand_Right, VEXOS_ARGRANGE);
    ErrorIf(type < JoystickButtonType_Down || type > JoystickButtonType_Top, VEXOS_ARGRANGE);

    JoystickButtonData data = Joystick_findButton(hand, type);
    return Button_new(&JoystickButton, stick->id, data.channel, data.button);
}

bool Joystick_getRawButton(Joystick* stick, JoystickHand hand, JoystickButtonType type) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    JoystickButtonData data = Joystick_findButton(hand, type);
    return GetJoystickDigital(stick->id, data.channel, data.button);
}

float Joystick_getAccelerometerX(Joystick* stick) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    return GetJoystickAccelerometer(stick->id, JOYSTICK_ACCEL_X) / MAX_JOYSTICK_ACCEL;
}

float Joystick_getAccelerometerY(Joystick* stick) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);

    return GetJoystickAccelerometer(stick->id, JOYSTICK_ACCEL_Y) / MAX_JOYSTICK_ACCEL;
}

void Joystick_setAccelerometerXDeadband(Joystick* stick, float lower, float upper) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);
    ErrorIf(lower < -1.0 || lower > 0, VEXOS_ARGRANGE);
    ErrorIf(upper > 1.0 || upper < 0, VEXOS_ARGRANGE);

    SetJoystickAccelDeadband(stick->id, JOYSTICK_ACCEL_X, 
                lower * MAX_JOYSTICK_ACCEL, upper * MAX_JOYSTICK_ACCEL);
}

void Joystick_setAccelerometerYDeadband(Joystick* stick, float lower, float upper) {
    ErrorIf(stick == NULL, VEXOS_ARGNULL);
    ErrorIf(lower < -1.0 || lower > 0, VEXOS_ARGRANGE);
    ErrorIf(upper > 1.0 || upper < 0, VEXOS_ARGRANGE);

    SetJoystickAccelDeadband(stick->id, JOYSTICK_ACCEL_Y, 
                lower * MAX_JOYSTICK_ACCEL, upper * MAX_JOYSTICK_ACCEL);
}
