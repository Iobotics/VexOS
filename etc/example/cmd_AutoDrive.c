//
//  cmd_AutoDrive.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(AutoDrive, { });

static void constructor(va_list argp) {
    require(&Drive);
}

static void initialize() { }

static void execute() {
    PrintToScreen("->AutoDrive execute\n");
}

static bool isFinished() {
    return true;
}

static void end() {
    Drive_setPower(0.0, 0.0);
}

static void interrupted() { }



