//
//  sys_Lift.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "robot.h"

// gear ratio conversion factor //
#define DEGREES_PER_TICK    ((360.0 * 12)/(TicksPerRev_IME_393HT * 84))

// PID constants //
#define Kp      0.5
#define Ki      0.0
#define Kd      0.3
#define THRESH  36

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Lift);

static LiftPosition currentPos = LiftPosition_Ground;
static bool         pidEnabled = false;

static void constructor() {
    DefineIntegratedMotorEncoderPID(MOTOR_LIFT_L, Kp, Ki, Kd, THRESH);
    DefineIntegratedMotorEncoderPID(MOTOR_LIFT_R, Kp, Ki, Kd, THRESH);
}

static void initDefaultCommand() { }

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static long getTicksForPosition(LiftPosition pos) {
    return (long) (pos * DEGREES_PER_TICK);
}

static void setPIDEnabled(bool flag) {
    if(pidEnabled == flag) return;
    if(flag) {
        long ticks = getTicksForPosition(currentPos);
        StartIntegratedMotorEncoderPID(MOTOR_LIFT_L, ticks);
        StartIntegratedMotorEncoderPID(MOTOR_LIFT_R, -ticks);
    } else {
        StopIntegratedMotorEncoderPID(MOTOR_LIFT_L);
        StopIntegratedMotorEncoderPID(MOTOR_LIFT_R);
    }
    pidEnabled = flag;
}

static void setPIDTicks(long ticks) {
    UpdateSetpointIntegratedMotorEncoderPID(MOTOR_LIFT_L, ticks);
    UpdateSetpointIntegratedMotorEncoderPID(MOTOR_LIFT_R, -ticks);
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
            setPIDTicks(getTicksForPosition(pos));
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
    setPIDEnabled(false);
    SetMotor(MOTOR_LIFT_L, power);
    SetMotor(MOTOR_LIFT_R, power);
    currentPos = LiftPosition_Free;
}

void Lift_resetEncoders() {
    long ticks = getTicksForPosition(LiftPosition_Ground);
    PresetIntegratedMotorEncoder(MOTOR_LIFT_L, ticks);
    PresetIntegratedMotorEncoder(MOTOR_LIFT_R, ticks);
    currentPos = LiftPosition_Ground;
}

bool Lift_getOnTarget() {
    return OnTargetIntegratedMotorEncoderPID(MOTOR_LIFT_L)
    && OnTargetIntegratedMotorEncoderPID(MOTOR_LIFT_R);
}

bool Lift_getResetSwitch() {
    return !GetDigitalInput(DIGITAL_LIFT_RESET);
}