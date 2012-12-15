//
//  sys_Drive.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Drive);

static DebugValue* lDrive;
static DebugValue* rDrive;

static void constructor(va_list argp) {
    lDrive = DebugValue_new("lDrive", DebugValueType_Float);
    rDrive = DebugValue_new("rDrive", DebugValueType_Float);
}

static void initDefaultCommand() {
    setDefaultCommand(Command_new(&DriveWithJoystick));
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void Drive_setPower(Power left, Power right) {
    // left motors //
    SetMotor(MOTOR_DRIVE_L_IME, left);
    SetMotor(MOTOR_DRIVE_L_1,   -left);
    SetMotor(MOTOR_DRIVE_L_2,   left);
    // right motors //
    SetMotor(MOTOR_DRIVE_R_IME, -right);
    SetMotor(MOTOR_DRIVE_R_1,   right);
    SetMotor(MOTOR_DRIVE_R_2,   -right);
    
    DebugValue_set(lDrive, left);
    DebugValue_set(rDrive, right);
}

void Drive_move(int inches) {
    // FIXME //
}

void Drive_turn(int degrees) {
    // FIXME //
}
