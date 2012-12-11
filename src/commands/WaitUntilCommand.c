//
//  WaitUntilCommand.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(WaitUntilCommand, {
    unsigned long gameTime;
});

static void constructor(va_list argp) {
    self->fields->gameTime = va_arg(argp, unsigned long);
    setArgs("%ld", self->fields->gameTime);
}

static void initialize() { }

static void execute() { }

static bool isFinished(Command* cmd) {
    return (GetMsClock() >= self->fields->gameTime);
}

static void end() { }

static void interrupted() { }
