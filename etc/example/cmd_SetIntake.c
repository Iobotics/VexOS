//
//  cmd_SetIntake.c
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

DeclareCommandClass(SetIntake, {
    IntakeDirection dir;
});

static void constructor(va_list argp) {
    self->fields->dir = va_arg(argp, IntakeDirection);
    setArgs("%s", (self->fields->dir == IntakeDirection_Stop)? "Stop":
                  (self->fields->dir == IntakeDirection_Suck)? "Suck":
                  "Blow");
    require(&Intake);
}

static void initialize() {
    Intake_setDirection(self->fields->dir);
}

static void execute() { }

static bool isFinished() {
    return true;
}

static void end() { }

static void interrupted() { }
