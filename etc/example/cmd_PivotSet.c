//
//  cmd_PivotSet.c
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

DeclareCommandClass(PivotSet, {
    PivotPosition pos;
});

static void constructor(va_list argp) {
    self->fields->pos = (PivotPosition) va_arg(argp, int);
    setArgs("%s", (self->fields->pos == PivotPosition_Down)? "Down": "Up");
    require(&Pivot);
}

static void initialize() {
    Pivot_setPosition(self->fields->pos);
}

static void execute() { }

static bool isFinished() {
    return true;
}

static void end() { }

static void interrupted() { }



