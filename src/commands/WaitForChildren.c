//
//  WaitForChildren.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/06/2012.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
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
