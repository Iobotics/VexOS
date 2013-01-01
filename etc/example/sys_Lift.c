//
//  sys_Lift.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "Robot.h"

// PID constants //
#define Kp      0.5
#define Ki      0.0
#define Kd      0.3
#define THRESH  36

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineSubsystem(Lift);

static LiftPosition currentPos = LiftPosition_Ground;
static MotorGroup*  left;
static MotorGroup*  right;
static DigitalIn*   resetSwitch;

static void constructor() {
    left  = MotorGroup_new("lift left");
    MotorGroup_addWithIME(left, "lift left", MOTOR_LIFT_L, MotorType_393_HS, false, I2c_1);
    MotorGroup_setOutputScaleFactor(left, 12.0 / 84.0);
    MotorGroup_setPID(left, Kp, Ki, Kd);

    right = MotorGroup_new("lift right");
    MotorGroup_addWithIME(right, "lift right", MOTOR_LIFT_R, MotorType_393_HS, true, I2c_2);
    MotorGroup_setOutputScaleFactor(right, 12.0 / 84.0);
    MotorGroup_setPID(right, Kp, Ki, Kd);

    resetSwitch = DigitalIn_newLimit("reset switch", DIGITAL_LIFT_RESET);
}

static void initialize() {
    MotorGroup_setPIDEnabled(left,  true);
    MotorGroup_setPIDEnabled(right, true);
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void Lift_setPosition(LiftPosition pos) {
    if(currentPos == pos) return;
    switch(pos) {
        case LiftPosition_Ground:
        case LiftPosition_Descore:
        case LiftPosition_Score:
            MotorGroup_setSetpoint(left,  pos);
            MotorGroup_setSetpoint(right, pos);
            break;
        case LiftPosition_Free:
        default:
            Lift_setPower(0);
            return;
    }
    currentPos = pos;
}

LiftPosition Lift_getPosition() {
    return currentPos;
}

void Lift_setPower(Power power) {
    MotorGroup_setPower(left,  power);
    MotorGroup_setPower(right, power);
    currentPos = LiftPosition_Free;
}

void Lift_resetEncoders() {
    MotorGroup_presetPosition(left,  LiftPosition_Ground);
    MotorGroup_presetPosition(right, LiftPosition_Ground);
    currentPos = LiftPosition_Ground;
}

bool Lift_getOnTarget() {
    return MotorGroup_onTarget(left) && MotorGroup_onTarget(right);
}

bool Lift_getResetSwitch() {
    return DigitalIn_get(resetSwitch);
}






