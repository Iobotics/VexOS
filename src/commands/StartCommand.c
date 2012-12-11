//
//  StartCommand.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(StartCommand, {
    Command* command;
});

static void constructor(va_list argp) {
    self->fields->command = va_arg(argp, Command*);
    setArgs("%s", Command_getName(self->fields->command));
}

static void initialize(Command* cmd) {
    Command_start(self->fields->command);
}

static void execute() { }

static bool isFinished() {
    return true;
}

static void end() { }

static void interrupted() { }
