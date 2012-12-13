//
//  Subsystem.c
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

#include "Error.h"
#include "Subsystem.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

extern Subsystem* const RobotSubsystems[];
List Subsystems;

void Subsystem_initialize() {
    int i = 0;
    Subsystem* sys;
    while((sys = RobotSubsystems[i++])) {
        if(sys->initialized) continue;
        sys->initialized = true;
        *(sys->selfPtr)  = sys;
        // call constructor //
        if(sys->constructor) sys->constructor();
        List_insertLast(&Subsystems, List_newNode(sys));
    }
}

Command* Subsystem_getDefaultCommand(Subsystem* sys) {
    if(!sys->initializedDefaultCommand) {
        sys->initializedDefaultCommand = true;
        if(sys->initDefaultCommand) sys->initDefaultCommand();
    }
    return sys->defaultCommand;
}

void Subsystem_setDefaultCommand(Subsystem* sys, Command* cmd) {
    if(cmd == NULL) sys->defaultCommand = NULL;
    else {
        ErrorMsgIf(!Command_doesRequireSubsystem(cmd, sys), VEXOS_ARGINVALID,
                   "Default command must require subsystem: %s", Command_getName(cmd));
        sys->defaultCommand = cmd;
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

String Subsystem_getName(Subsystem* sys) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    return sys->name;
}

bool Subsystem_isInitialized(Subsystem* sys) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);

    return sys->initialized;
}

Command* Subsystem_getCurrentCommand(Subsystem* sys) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    return sys->currentCommand;
}
