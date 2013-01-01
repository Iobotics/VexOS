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
//  --------------------------------------------------------------------------
//
//  Some algorithms, structures and code organization used by this file have
//  been adapted from the WPILibJ software package. Those portions are:
//
//  Copyright (c) FIRST 2008-2012. All rights reserved.
//  
//  WPILibJ elements are distributed in this program in accordance to their 
//  original FIRST BSD License, available as 'BSD_License_for_WPILib_code.txt' 
//  present in this distribution.
//

#include "Error.h"
#include "Subsystem.h"
#include "Hardware.h"
#include "Device.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

extern struct Robot Robot;
List Subsystems;

// hidden method, not in header //
void Subsystem_construct() {
    int i = 0;
    Subsystem* sys;
    while((sys = Robot.subsystems[i++])) {
        if(sys->initialized) continue;
        *(sys->selfPtr)  = sys;
        // call constructor //
        Device_setSubsystem(sys);
        if(sys->constructor) sys->constructor();
        Device_setSubsystem(NULL);
        sys->initialized = true;
        List_insertLast(&Subsystems, List_newNode(sys));
    }
}

// hidden method, not in header //
void Subsystem_initialize() {
    ListNode* node = Subsystems.firstNode;
    while(node != NULL) {
        Subsystem* sys = node->data;
        if(sys->initialize) sys->initialize();
        node = node->next;
    }
}

Command* Subsystem_getDefaultCommand(Subsystem* sys) {
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
