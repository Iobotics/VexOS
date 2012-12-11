//
//  Subsystem.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Error.h"
#include "Subsystem.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

List Subsystems;

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

void Subsystem_register(Subsystem* sys, ...) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    if(sys->initialized) return;
    sys->initialized = true;
    *(sys->selfPtr)  = sys;
    // call constructor //
    va_list argp;
    va_start(argp, sys);
    if(sys->constructor) sys->constructor(argp);
    va_end(argp);
    List_insertLast(&Subsystems, List_newNode(sys));
}

String Subsystem_getName(Subsystem* sys) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    return sys->name;
}

bool Subsystem_isRegistered(Subsystem* sys) {
    return sys->initialized;
}

Command* Subsystem_getCurrentCommand(Subsystem* sys) {
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    return sys->currentCommand;
}
