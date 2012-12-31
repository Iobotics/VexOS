//
//  CommandGroup.h
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/08/2012.
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
    float           timeout;
};

bool        CommandGroup_isGroup(Command* cmd);
bool        CommandGroup_isInterruptible(Command* group);
Command*    CommandGroup_getCurrentCommand(Command* group);
const List* CommandGroup_getChildList(Command* group);

#endif
