//
//  Scheduler.c
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

#include "Scheduler.h"
#include "Button.h"
#include "Command.h"
#include "CommandGroup.h"
#include "Subsystem.h"
#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static ListNode* getNode(void*);
static void freeNode(ListNode*);
static void addRunningNode(ListNode*);
static void removeRunningNode(ListNode*);

static List runningList;
static List queuedList;
static List buttonList;
static List nodeCache;

typedef struct {
    unsigned int objectId;
    Color        color;
} DisplayLine;

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

static void freeNode(ListNode* node) {
    List_insertLast(&nodeCache, node);
}

static void addRunningNode(ListNode* node) {
    static bool adding = false;
    
    // validate //
    if(node == NULL) return;
    Command* cmd = (Command*) node->data;
    if(cmd == NULL) { freeNode(node); return; }
    
    if(adding) {
        Info("Cannot start command during cancel(). Ignoring: %s", Command_getName(cmd));
        freeNode(node);
        return;
    }
    
    // make sure command is not already running //
    ListNode* xnode = runningList.firstNode;
    while(xnode != NULL) {
        if((Command*)xnode->data == cmd) {
            freeNode(node);
            return;
        }
        xnode = xnode->next;
    }
    
    // check that required Subsystems are available //
    Command* current;
    xnode = cmd->requiresList.firstNode;
    while(xnode != NULL) {
        current = ((Subsystem*) xnode->data)->currentCommand;
        if(current && !current->interruptible) {
            freeNode(node);
            return;
        }
        xnode = xnode->next;
    }
    
    // claim required Subsystems //
    adding = true;
    xnode = cmd->requiresList.firstNode;
    while(xnode != NULL) {
        Subsystem* sys = (Subsystem*) xnode->data;
        current = sys->currentCommand;
        if(current) {
            Command_cancel(current);
            // find the node with the command and remove it //
            ListNode* xnode2 = runningList.firstNode;
            while(xnode2 != NULL) {
                if((Command*)xnode2->data == current) {
                    removeRunningNode(xnode2);
                    break;
                }
                xnode2 = xnode2->next;
            }
        }
        sys->currentCommand = cmd;
        xnode = xnode->next;
    }
    adding = false;
    
    // add to command list //
    List_insertLast(&runningList, node);
    Command_startRunning(cmd);
}

static void removeRunningNode(ListNode* node) {
    if(node == NULL) return;
    Command* cmd = (Command*) node->data;
    if(cmd == NULL) return;
    
    // remove from linked list //
    freeNode(node);
    
    // clear all owned Subsystems //
    ListNode* xnode = cmd->requiresList.firstNode;
    while(xnode != NULL) {
        ((Subsystem*) xnode->data)->currentCommand = NULL;
        xnode = xnode->next;
    }
    
    Command_removed(cmd);
}

static void printCommands(Command* cmd, DisplayLine* cache, unsigned char* line, int indent,
    Rect innerRect, unsigned char height, unsigned char width) 
{
    // make sure we don't overflow the window //
    if(*line >= height) return;

    // NULL indicates root node, print scheduler Commands //
    if(!cmd) {
        ListNode* node = runningList.firstNode;
        while(node != NULL) {
            printCommands(node->data, cache, line, indent, innerRect, height, width);
            node = node->next;
        }
        return;
    }
    
    // determine line color //
    Color color = Color_Black;
    if(cmd->startTime == -1) {
        color = Color_DarkYellow;
    } else if(Command_timeSinceInitialized(cmd) < 1000) {
        color = Color_DarkGreen;
    }

    // print this Command if changed //
    if(cmd->objectId != cache[*line].objectId || color != cache[*line].color) {
        // compute display characteristics: position //
        unsigned char top    = innerRect.top + *line;
        unsigned char left   = innerRect.left;
        unsigned char xwidth = width - indent;
        // print command and increment line counter //
        PrintTextToGD(top, left, color, "%*s%-*.*s\n", indent, "", xwidth, xwidth, 
                      Command_getName(cmd));
        cache[*line] = (DisplayLine) { .objectId = cmd->objectId, .color = color };
    }
    (*line)++;
    // test for CommandGroup //
    if(CommandGroup_isGroup(cmd)) {
        // print children //
        ListNode* node = CommandGroup_getChildList(cmd)->firstNode;
        while(node != NULL) {
            GroupEntry* entry = node->data;
            printCommands(entry->command, cache, line, indent + 2, innerRect, height, width);
            node = node->next;
        }
        // print current node last //
        Command* xcmd = CommandGroup_getCurrentCommand(cmd);
        if(xcmd) printCommands(xcmd, cache, line, indent + 2, innerRect, height, width);
    }
}

static void updateWindow(Window* win, bool full) {
    static DisplayLine*  lineCache = NULL;
    static unsigned char lastCount = 0;

    Rect innerRect       = Window_getInnerRect(win);
    unsigned char height = Window_getHeight(win);
    unsigned char width  = Window_getWidth(win);
    
    // create the line ID cache if it doesn't exist //
    if(!lineCache) {
        lineCache = malloc(height * sizeof(DisplayLine));
    }
    // clear out ID cache to force a full redraw //
    if(full) {
        memset(lineCache, 0, height * sizeof(DisplayLine));
    }

    // recursively print nodes //
    unsigned char line = 0;
    printCommands(NULL, lineCache, &line, 0, innerRect, height, width);
    
    // check to see if we printed less than last time //
    if(line < lastCount) {
        // clear remaining lines //
        ClearGD(innerRect.top + line, innerRect.left, innerRect.bottom, 
                innerRect.right, false);
    }
    lastCount = line;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Scheduler_add(Command* cmd) {
    if(cmd == NULL) return;
    List_insertLast(&queuedList, getNode(cmd));
}

void Scheduler_addButtonScheduler(ButtonScheduler* sched) {
    if(sched == NULL) return;
    List_insertLast(&buttonList, getNode(sched));
}

void Scheduler_removeButtonSchedulers(Button* button) {
    if(button == NULL) return;
    ListNode* node = buttonList.firstNode;
    while(node != NULL) {
        ButtonScheduler* sched = node->data;
        if(Button_getSchedulerButton(sched) == button) {
            node = List_remove(node);
            Button_freeButtonScheduler(sched);
            continue;
        }
        node = node->next;
    }
}

void Scheduler_run() {
    // handle buttons (go backwards to preserve priority) //
    ListNode* node = buttonList.lastNode;
    while(node != NULL) {
        Button_executeScheduler((ButtonScheduler*) node->data);
        node = node->prev;
    }
    
    // loop through commands to execute //
    node = runningList.firstNode;
    ListNode* temp;
    while(node != NULL) {
        temp = node->next;
        if(!Command_run((Command*) node->data)) {
            removeRunningNode(node);
        }
        node = temp;
    }
    
    // add queued stuff //
    node = queuedList.firstNode;
    while(node != NULL) {
        temp = node->next;
        addRunningNode(node);
        node = temp;
    }
    
    // add in the defaults //
    node = Subsystems.firstNode;
    while(node != NULL) {
        Subsystem* sys = (Subsystem*) node->data;
        if(!sys->currentCommand) {
            Command* xcmd = Subsystem_getDefaultCommand(sys);
            addRunningNode(getNode(xcmd));
        }
        node = node->next;
    }
}

/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

Window* Scheduler_getWindow() {
    static Window* window = NULL;
    if(window) return window;
    window = Window_new("Running Commands", &updateWindow);
    Window_setSize(window, 40, 17);
    return window;
}

