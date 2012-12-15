//
//  cmd_DriveWithJoystick.c
//  VexOS
//
//  Created by Jeff Malins on 12/5/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(DriveWithJoystick, { });

static void constructor(va_list argp) {
    require(&Drive);
    setTimeout(2000);
}

static void initialize() { }

static void execute() {
    Drive_setPower(GetJoystickAnalog(1, JoystickChannelType_LeftY),
                   GetJoystickAnalog(1, JoystickChannelType_RightY));
}

static bool isFinished() {
    return isTimedOut();
}

static void end() {
    Drive_setPower(0, 0);
}

static void interrupted() { }
