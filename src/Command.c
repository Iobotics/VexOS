//
//  Command.c
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

#include "API.h"

#include "Command.h"
#include "CommandClass.h"
#include "CommandGroup.h"
#include "Scheduler.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static unsigned int lastObjectId = 0;

static void callConstructorMethod(Command* cmd, void (method)(va_list), va_list argp) {
    if(!method) return;
    Command* old = *(cmd->class->selfPtr);
    *(cmd->class->selfPtr) = cmd;
    method(argp);
    *(cmd->class->selfPtr) = old;
}

static void callGroupConstructorMethod(Command* cmd, void (method)(va_list), va_list argp) {
    if(!method) return;
    Command* old  = *(cmd->class->selfPtr);
    Command* old2 = *(cmd->class->groupSelfPtr);
    *(cmd->class->selfPtr)      = cmd;
    *(cmd->class->groupSelfPtr) = cmd;
    method(argp);
    *(cmd->class->selfPtr)      = old;
    *(cmd->class->groupSelfPtr) = old2;
}

static void callVoidMethod(Command* cmd, void (method)()) {
    if(!method) return;
    Command* old = *(cmd->class->selfPtr);
    *(cmd->class->selfPtr) = cmd;
    method();
    *(cmd->class->selfPtr) = old;
}

static bool callBoolMethod(Command* cmd, bool (method)()) {
    if(!method) return true;
    Command* old = *(cmd->class->selfPtr);
    *(cmd->class->selfPtr) = cmd;
    bool ret = method();
    *(cmd->class->selfPtr) = old;
    return ret;
}

static void initializeCommandClass(CommandClass* class) {
    // if a group subclass, clone things from the prototype CommandGroup //
    if(class->groupConstructor) {
        class->selfPtr     = CommandGroup.selfPtr;
        class->fieldSize   = CommandGroup.fieldSize;
        class->constructor = CommandGroup.constructor;
        class->destructor  = CommandGroup.destructor;
        class->initialize  = CommandGroup.initialize;
        class->execute     = CommandGroup.execute;
        class->isFinished  = CommandGroup.isFinished;
        class->end         = CommandGroup.end;
        class->interrupted = CommandGroup.interrupted;
        *(class->groupSelfPtr) = NULL;
    }
    // normal initialization //
    class->initialized = true;
    *(class->selfPtr)  = NULL;
    class->lastInstanceId = 0;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

bool Command_run(Command* cmd) {
    if(!cmd->runWhenDisabled && !cmd->parent && !IsEnabled()) {
        Command_cancel(cmd);
    }
    if(cmd->status & CommandStatus_Cancelled) {
        return false;
    }
    if(!(cmd->status & CommandStatus_Initialized)) {
        cmd->status |= CommandStatus_Initialized;
        cmd->startTime = GetMsClock();
        Debug("Initialize:  %s", Command_getName(cmd));
        callVoidMethod(cmd, cmd->class->initialize);
    }
    Debug("Execute:     %s", Command_getName(cmd));
    callVoidMethod(cmd, cmd->class->execute);
    return !callBoolMethod(cmd, cmd->class->isFinished);
}

void Command_removed(Command* cmd) {
    if(cmd->status & CommandStatus_Initialized) {
        if(cmd->status & CommandStatus_Cancelled) {
            Debug("Interrupted: %s", Command_getName(cmd));
            callVoidMethod(cmd, cmd->class->interrupted);
        } else {
            Debug("End:         %s", Command_getName(cmd));
            callVoidMethod(cmd, cmd->class->end);
        }
    }
    cmd->status &= ~(CommandStatus_Initialized
                     | CommandStatus_Cancelled
                     | CommandStatus_Running
                     );
}

void Command_setCancelled(Command* cmd) {
    if(cmd->status & CommandStatus_Running) {
        cmd->status |= CommandStatus_Cancelled;
    }
}

void Command_setParent(Command* cmd, Command* group) {
    ErrorMsgIf(cmd->parent != NULL, VEXOS_OPINVALID, "Command already has a CommandGroup: %s\n", Command_getName(cmd));
    cmd->status |= CommandStatus_Locked;
    cmd->parent = group;
}

void Command_startRunning(Command* cmd) {
    cmd->status |= CommandStatus_Running;
    cmd->startTime = -1;
    
}

/********************************************************************
 * Protected API: Implicit Scope Methods                            *
 ********************************************************************/

void Command_setvName(Command* cmd, String fmt, va_list argp) {
    ErrorIf(fmt == NULL, VEXOS_ARGNULL);
    
    char* old = (char*) cmd->name;
    vasprintf((char**) &cmd->name, fmt, argp);
    if(old) free(old);
}

void Command_setvArgs(Command* cmd, String fmt, va_list argp) {
    ErrorIf(fmt == NULL, VEXOS_ARGNULL);
    
    char* old = (char*) Command_getName(cmd);
    char* xfmt;
    asprintf(&xfmt, "%s(%s)", old, fmt);
    // print new name //
    vasprintf((char**) &cmd->name, xfmt, argp);
    free(xfmt);
    free(old);
}

void Command_require(Command* cmd, Subsystem* sys) {
    ErrorMsgIf(cmd->status & CommandStatus_Locked, VEXOS_OPINVALID,
               "Cannot add Subsystem, Command is locked: %s", Command_getName(cmd));
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!Subsystem_isInitialized(sys), VEXOS_OPINVALID, "Subsystem is not initialized: %s",
               Subsystem_getName(sys));
                
    if(!Command_doesRequireSubsystem(cmd, sys)) {
        List_insertLast(&cmd->requiresList, List_newNode(sys));
    }
}

void Command_setTimeout(Command* cmd, unsigned long time) {
    cmd->timeout = time;
}

bool Command_isTimedOut(Command* cmd) {
    return (cmd->timeout != -1) &&
        (Command_timeSinceInitialized(cmd) >= cmd->timeout);
}

void Command_setInterruptible(Command* cmd, bool inter) {
    cmd->interruptible = inter;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Command* Command_new(CommandClass* class, ...) {
    ErrorIf(class == NULL, VEXOS_ARGNULL);
    
    // check the class //
    if(!class->initialized) {
        initializeCommandClass(class);
    }
    // create the Command //
    Command* cmd = malloc(sizeof(Command));
    cmd->class           = class;
    cmd->objectId        = ++lastObjectId;
    cmd->instanceId      = ++class->lastInstanceId;
    cmd->name            = NULL;
    cmd->parent          = NULL;
    cmd->status          = 0;
    cmd->startTime       = -1;
    cmd->timeout         = -1;
    cmd->interruptible   = true;
    cmd->runWhenDisabled = false;
    memset(&cmd->requiresList, 0, sizeof(List));
    // build the fields //
    cmd->fields = (class->fieldSize > 0)? malloc(class->fieldSize): NULL;
    // call the constructor //
    va_list argp;
    va_start(argp, class);
    callConstructorMethod(cmd, class->constructor, argp);
    // if a group subsclass, call the secondary constructor //
    if(class->groupConstructor) {
        callGroupConstructorMethod(cmd, class->groupConstructor, argp);
    }
    va_end(argp);
    return cmd;
}

Command* Command_delete(Command* cmd) {
    if(!cmd) return NULL;
    callVoidMethod(cmd, cmd->class->destructor);
    if(cmd->name) free((void*) cmd->name);
    if(cmd->fields) free(cmd->fields);
    free(cmd);
    return NULL;
}

void Command_start(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(cmd->parent != NULL, VEXOS_ARGINVALID,
               "Cannot start Command in a CommandGroup: %s", Command_getName(cmd));
    
    cmd->status |= CommandStatus_Locked;
    Scheduler_add(cmd);
}

void Command_cancel(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(cmd->parent != NULL, VEXOS_ARGINVALID,
               "Cannot cancel Command in a CommandGroup: %s", Command_getName(cmd));
    
    Command_setCancelled(cmd);
}

String Command_getName(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);

    if(!cmd->name) {
        asprintf((char**) &cmd->name, "%s[%d]", cmd->class->name, cmd->instanceId);
    }
    return cmd->name;
}

CommandClass* Command_getClass(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return (CommandClass*) cmd->class;
}

Command* Command_getParent(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return cmd->parent;
}

CommandStatus Command_GetStatus(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return cmd->status;
}

bool Command_isInterruptible(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    // special handling for CommandGroup //
    if(CommandGroup_isGroup(cmd)) {
        CommandGroup_isInterruptible(cmd);
    }
    return cmd->interruptible;
}

bool Command_runWhenDisabled(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return cmd->runWhenDisabled;
}

void Command_setRunWhenDisabled(Command* cmd, bool run) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    cmd->runWhenDisabled = run;
}

bool Command_doesRequireSubsystem(Command* cmd, Subsystem* sys) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    ErrorIf(sys == NULL, VEXOS_ARGNULL);
    
    // loop through subsystems //
    ListNode* node = cmd->requiresList.firstNode;
    while(node != NULL) {
        if(((const Subsystem*) node->data) == sys) return true;
        node = node->next;
    }
    return false;
}

unsigned long Command_timeSinceInitialized(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return (GetMsClock() - cmd->startTime);
}
