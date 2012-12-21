//
//  cmd_DriveWithJoystick.c
//  VexOS
//
//  Created by Jeff Malins on 12/5/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(DriveWithJoystick, { });

static Joystick* stick;

static void constructor(va_list argp) {
    stick = Joystick_new(1);
    require(&Drive);
}

static void initialize() { }

static void execute() {
    Drive_setPower(Joystick_getY(stick, JoystickHand_Left),
                   Joystick_getY(stick, JoystickHand_Right));
}

static bool isFinished() {
    return isTimedOut();
}

static void end() {
    Drive_setPower(0, 0);
}

static void interrupted() { }




