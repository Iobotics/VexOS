//
//  CommandGroup.h
//  VexOS
//
//  Created by Jeff Malins on 12/8/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _CommandGroup_h
#define _CommandGroup_h

#include "VexOS.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

// group entry type code //
typedef enum {
    GroupEntryState_InSequence,
    GroupEntryState_BranchPeer,
    GroupEntryState_BranchChild
} GroupEntryState;

// group entry structure //
typedef struct GroupEntry GroupEntry;
struct GroupEntry {
    Command*        command;
    GroupEntryState state;
    unsigned long   timeout;
};

bool CommandGroup_isInterruptible(Command*);
Command* CommandGroup_getCurrentCommand(Command*);
List* CommandGroup_getChildList(Command*);

#endif
