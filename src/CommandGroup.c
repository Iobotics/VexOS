//
//  CommandGroup.c
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

#include "Command.h"
#include "CommandClass.h"
#include "CommandGroup.h"
#include "Error.h"

/********************************************************************
 * Class Declaration                                                *
 ********************************************************************/

DeclareCommandClass(CommandGroup, {
    String    name;
    List      commands;
    List      children;
    ListNode* currentNode;
    bool      done;
});

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

// static pool of nodes //
static List nodeCache;

static ListNode* getNode(void* data) {
    ListNode* node = nodeCache.firstNode;
    if(node != NULL) {
        List_remove(node);
        node->data = data;
    } else {
        node = List_newNode(data);
    }
    return node;
}

static ListNode* freeNode(ListNode* node) {
    ListNode* next = List_remove(node);
    List_insertLast(&nodeCache, node);
    return next;
}

static void addEntryNode(Command* group, Command* cmd, GroupEntryState state, unsigned long timeout) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->class != &CommandGroup, VEXOS_ARGINVALID,
               "Command is not a CommandGroup: %s", Command_getName(group));
    ErrorMsgIf(group->status & CommandStatus_Locked, VEXOS_OPINVALID,
               "Cannot add Command, CommandGroup is locked: %s", Command_getName(cmd));
    
    // process the entry //
    GroupEntry* entry = (GroupEntry*) malloc(sizeof(GroupEntry));
    entry->command = cmd;
    entry->state   = state;
    entry->timeout = timeout;
    // add to command list //
    List_insertLast(&group->fields->commands, getNode(entry));
    cmd->parent = group;
    // add requirements to group //
    ListNode* node = cmd->requiresList.firstNode;
    while(node != NULL) {
        Command_require(group, (Subsystem*) node->data);
        node = node->next;
    }
}

static bool isEntryTimedOut(GroupEntry* entry) {
    if(entry->timeout == -1) {
        return false;
    } else {
        unsigned long time = Command_timeSinceInitialized(entry->command);
        return (time == 0)? false : (time >= entry->timeout);
    }
}

// cancel active children with the same requirements as specifed command //
static void cancelConflicts(Command* command) {
    ListNode* node = self->fields->children.firstNode;
    ListNode* temp = NULL;
    while(node != NULL) {
        temp = NULL;
        GroupEntry* entry = (GroupEntry*) node->data;
        // get command requirements //
        ListNode* reqNode = command->requiresList.firstNode;
        while(reqNode != NULL) {
            Subsystem* sys = reqNode->data;
            // remove child if it requires same subsystem as command //
            if(Command_doesRequireSubsystem(entry->command, sys)) {
                Command_setCancelled(entry->command);
                Command_removed(entry->command);
                temp = freeNode(node);
                break;
            }
            reqNode = reqNode->next;
        }
        node = (temp)? temp: node->next;
    }
}

/********************************************************************
 * Class Implementation                                             *
 ********************************************************************/

static void constructor(va_list argp) {
    self->fields->name = va_arg(argp, String);
    memset(&self->fields->commands, 0, sizeof(List));
    memset(&self->fields->children, 0, sizeof(List));
    self->fields->currentNode = NULL;
    self->fields->done        = false;
    setName("%s", self->fields->name);
}

static void initialize() {
    self->fields->currentNode = NULL;
    self->fields->done        = false;
}

static void execute() {
    GroupEntry* entry = NULL;
    Command*    cmd   = NULL;
    
    bool firstRun = false;
    if(!self->fields->done && self->fields->currentNode == NULL) {
        firstRun = true;
        self->fields->currentNode = self->fields->commands.firstNode;
    }
    
    while(self->fields->currentNode != NULL) {
        if(cmd != NULL) {
            if(isEntryTimedOut(entry)) {
                Command_setCancelled(cmd);
            }
            if(Command_run(cmd)) {
                break;
            } else {
                Command_removed(cmd);
                self->fields->currentNode = self->fields->currentNode->next;
                if(!self->fields->currentNode) self->fields->done = true;
                firstRun = true;
                cmd      = NULL;
                continue;
            }
        }
        
        entry = (GroupEntry*) self->fields->currentNode->data;
        cmd   = NULL;
        switch (entry->state) {
            case GroupEntryState_InSequence:
                cmd = entry->command;
                if(firstRun) {
                    Command_startRunning(cmd);
                    cancelConflicts(cmd);
                }
                firstRun = false;
                break;
            case GroupEntryState_BranchPeer:
                // this seems to never be run //
                self->fields->currentNode = self->fields->currentNode->next;
                if(!self->fields->currentNode) self->fields->done = true;
                Command_start(entry->command);
                break;
            case GroupEntryState_BranchChild:
                self->fields->currentNode = self->fields->currentNode->next;
                if(!self->fields->currentNode) self->fields->done = true;
                cancelConflicts(entry->command);
                Command_startRunning(entry->command);
                // put a copy of the entry on the child list //
                List_insertLast(&self->fields->children, getNode(entry));
                break;
        }
    }
    
    // run children on the child list //
    ListNode* node = self->fields->children.firstNode;
    while(node != NULL) {
        entry = (GroupEntry*) node->data;
        if(isEntryTimedOut(entry)) {
            Command_setCancelled(entry->command);
        }
        if(!Command_run(entry->command)) {
            Command_removed(entry->command);
            node = freeNode(node);
            continue;
        }
        node = node->next;
    }
}

static void end() {
    // cancel the current command //
    Command* cmd = NULL;
    if(self->fields->currentNode != NULL) {
        cmd = ((GroupEntry*) self->fields->currentNode->data)->command;
        Command_setCancelled(cmd);
        Command_removed(cmd);
    }
    
    // cancel and remove all children //
    ListNode* node = self->fields->children.firstNode;
    while(node != NULL) {
        cmd = ((GroupEntry*) node->data)->command;
        Command_setCancelled(cmd);
        Command_removed(cmd);
        node = freeNode(node);
    }
}

static bool isFinished() {
    return (self->fields->done) && (self->fields->children.nodeCount == 0);
}

static void interrupted() {
    end();
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

bool CommandGroup_isInterruptible(Command* group) {
    // is the group interruptible (not sure how this is set if we don't allow subclassing) //
    if(!group->interruptible) return false;
    
    // see if the current node is interruptible //
    Command* cmd;
    if(group->fields->currentNode != NULL) {
        cmd = ((GroupEntry*) group->fields->currentNode->data)->command;
        if(!cmd->interruptible) return false;
    }
    
    // check for an uninterruptible child //
    ListNode* node = group->fields->children.firstNode;
    while(node != NULL) {
        cmd = ((GroupEntry*) node->data)->command;
        if(!cmd->interruptible) return false;
        node = node->next;
    }
    return true;
}

Command* CommandGroup_getCurrentCommand(Command* group) {
    ListNode* node = group->fields->currentNode;
    return (node)? ((GroupEntry*) node->data)->command: NULL;
}

List* CommandGroup_getChildList(Command* group) {
    return &group->fields->children;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void CommandGroup_addSequential(Command* group, Command* cmd) {
    ErrorEntryPoint();
    CommandGroup_addSequentialWithTimeout(group, cmd, -1);
    ErrorEntryClear();
}

void CommandGroup_addSequentialWithTimeout(Command* group, Command* cmd, unsigned long timeout) {
    ErrorEntryPoint();
    addEntryNode(group, cmd, GroupEntryState_InSequence, timeout);
    ErrorEntryClear();
}

void CommandGroup_addParallel(Command* group, Command* cmd) {
    ErrorEntryPoint();
    CommandGroup_addParallelWithTimeout(group, cmd, -1);
    ErrorEntryClear();
}

void CommandGroup_addParallelWithTimeout(Command* group, Command* cmd, unsigned long timeout) {
    ErrorEntryPoint();
    addEntryNode(group, cmd, GroupEntryState_BranchChild, timeout);
    ErrorEntryClear();
}
