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

DefineCommandClass(DriveWithJoystick, { });

static void constructor(va_list argp) {
    require(&Drive);
}

static void initialize() { }

static void execute() {
    Drive_setPower(Joystick_getY(joystick, JoystickHand_Left),
                   Joystick_getY(joystick, JoystickHand_Right));
}

static bool isFinished() {
    return isTimedOut();
}

static void end() {
    Drive_setPower(0, 0);
}

static void interrupted() { }





