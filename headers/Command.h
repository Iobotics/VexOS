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
    float               startTime;
    float               timeout;
    bool                interruptible;
    bool                runWhenDisabled;
    List                requiresList;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Command_constructor(Command* cmd);
bool Command_run(Command* cmd);
void Command_startRunning(Command* cmd);
void Command_removed(Command* cmd);
void Command_setCancelled(Command* cmd);

void Command_setvName(Command* cmd, String fmtString, va_list argp);
void Command_setvArgs(Command* cmd, String fmtString, va_list argp);
void Command_require(Command* cmd, Subsystem* sys);
void Command_setTimeout(Command* cmd, float timeoutSec);
bool Command_isTimedOut(Command* cmd);
void Command_setInterruptible(Command* cmd, bool value);
void Command_checkInstance(Command* cmd, CommandClass* class);

#endif // _Command_h
