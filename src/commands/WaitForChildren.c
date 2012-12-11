//
//  WaitForChildren.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "CommandGroup.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(WaitForChildren, { });

static void constructor(va_list argp) { }

static void initialize() { }

static void execute() { }

static bool isFinished() {
    Command* parent = Command_getParent(self);
    if(!parent || parent->class != &CommandGroup) return true;
    List* children = CommandGroup_getChildList(parent);
    return (children->nodeCount == 0);
}

static void end() { }

static void interrupted() { }
