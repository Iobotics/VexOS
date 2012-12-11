//
//  Command.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
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
