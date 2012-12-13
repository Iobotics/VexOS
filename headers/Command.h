//
//  Command.h
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

#ifndef _Command_h
#define _Command_h

#include "VexOS.h"

/********************************************************************
 * Command Structure                                                *
 ********************************************************************/

struct Command {
    // public members //
    struct Fields*      fields;
    const CommandClass* class;
    // hidden infrastructure fields //
    unsigned int        objectId;
    unsigned int        instanceId;
    String              name;
    Command*            parent;
    CommandStatus       status;
    unsigned long       startTime;
    unsigned long       timeout;
    bool                interruptible;
    bool                runWhenDisabled;
    List                requiresList;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Command_constructor(Command*);
bool Command_run(Command*);
void Command_startRunning(Command*);
void Command_removed(Command*);
void Command_setCancelled(Command*);

void Command_require(Command*, Subsystem*);
void Command_setTimeout(Command*, unsigned long time);
bool Command_isTimedOut(Command*);
void Command_setInterruptible(Command*, bool);

#endif // _Command_h
