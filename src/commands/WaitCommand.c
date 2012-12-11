//
//  WaitCommand.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(WaitCommand, { });

static void constructor(va_list argp) {
    unsigned long timeout = va_arg(argp, unsigned long);
    setTimeout(timeout);
    setArgs("%ld", timeout);
}

static void initialize() { }

static void execute() { }

static bool isFinished() {
    return Command_isTimedOut(self);
}

static void end() { }

static void interrupted() { }
