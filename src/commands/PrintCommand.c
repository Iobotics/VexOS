//
//  PrintCommand.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(PrintCommand, {
    String msg;
});

static void constructor(va_list argp) {
    self->fields->msg = va_arg(argp, String);
    setArgs("\"%s\"", self->fields->msg);
}

static void initialize() {
    Info("%s\n", self->fields->msg);
}

static void execute() { }

static bool isFinished() {
    return true;
}

static void end() { }

static void interrupted() { }
