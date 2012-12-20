//
//  sys_Drive.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "Hardware.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Drive);

static MotorGroup* leftDrive;
static MotorGroup* rightDrive;

static DebugValue* lDebug;
static DebugValue* rDebug;

static void constructor(va_list argp) {
    leftDrive  = MotorGroup_new("left drive");
    MotorGroup_addMotor(leftDrive, Motor_new("left top", MOTOR_DRIVE_L_IME, MotorType_393_HS));
    MotorGroup_addMotor(leftDrive, Motor_new("left middle", MOTOR_DRIVE_L_1, MotorType_393_HS));
    MotorGroup_addMotor(leftDrive, Motor_new("left bottom", MOTOR_DRIVE_L_2, MotorType_393_HS));

    rightDrive = MotorGroup_new("right drive");
    MotorGroup_addMotor(rightDrive, Motor_new("right top", MOTOR_DRIVE_R_IME, MotorType_393_HS));
    MotorGroup_addMotor(rightDrive, Motor_new("right middle", MOTOR_DRIVE_R_1, MotorType_393_HS));
    MotorGroup_addMotor(rightDrive, Motor_new("right bottom", MOTOR_DRIVE_R_2, MotorType_393_HS));

    lDebug = DebugValue_new("lDrive", DebugValueType_Float);
    rDebug = DebugValue_new("rDrive", DebugValueType_Float);
}

static void initDefaultCommand() {
    setDefaultCommand(Command_new(&DriveWithJoystick));
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void Drive_setPower(Power left, Power right) {
    MotorGroup_set(leftDrive,  left);
    MotorGroup_set(rightDrive, right);
    
    DebugValue_set(lDebug, left);
    DebugValue_set(rDebug, right);
}

void Drive_move(int inches) {
    // FIXME //
}

void Drive_turn(int degrees) {
    // FIXME //
}




