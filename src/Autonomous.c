//
//  Autononous.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/14/2012.
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

#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static List      autonomousPrograms;
static ListNode* activeProgram;
static bool      autoChanged;

static void autoStartHandler(EventType type, void* state) {
    Command* autop = Autonomous_getSelectedProgram();
    if(autop != NULL) {
        Command_start(autop);
    }
}

static void setActiveProgram(ListNode* node) {
    activeProgram = node;
    autoChanged   = true;
    // store in global data //
    GlobalData(GLOBALDATA_AUTO_PROGRAM) = (node)? (List_indexOfNode(node) + 1): 0;
    // if there is a program, register the event to run it //
    if(activeProgram) {
        VexOS_addEventHandler(EventType_AutonomousStart, autoStartHandler, NULL);
    }
}

static void updateWindow(Window* win, bool full) {
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);
    
    if(!full && !autoChanged) return;
    ClearGD(top, left, innerRect.bottom, innerRect.right, false);
    
    Command* selected = Autonomous_getSelectedProgram();
    int selLine = 0;
    
    // print the (none) choice //
    PrintTextToGD(top, left + 1, Color_Black, "(none)\n");
    if(selected == NULL) selLine = top;
    top++;
    
    // print the stored programs //
    ListNode* node = autonomousPrograms.firstNode;
    while(node != NULL) {
        Command* cmd = node->data;
        PrintTextToGD(top, left + 1, Color_Black, "%.*s\n", width, Command_getName(cmd));
        if(selected == cmd) selLine = top;
        top++;
        node = node->next;
    }
    
    // print the selection mark //
    PrintTextToGD(selLine, left, Color_Black, ">\n");
    autoChanged = false;
}

static bool getLCDStatus(LCDScreen* screen) {
    return autonomousPrograms.nodeCount > 0;
}

static void drawLCDScreen(LCDScreen* screen, LCDButtonType pressed) {
    LCD* lcd = LCDScreen_getLCD(screen);
    LCD_setText(lcd, 1, LCDTextOptions_Centered, "Auto Program");
    
    // move the selection //
    if(pressed == LCDButtonType_Right) {
        if(activeProgram == NULL) {
            setActiveProgram(autonomousPrograms.firstNode);
        } else if(activeProgram->next != NULL) {
            setActiveProgram(activeProgram->next);
        }
        PrintToScreen("right\n");
    } else if(pressed == LCDButtonType_Left) {
        if(activeProgram != NULL) {
            setActiveProgram(activeProgram->prev);
        }
        PrintToScreen("left\n");
    }
    
    // print the program choice to LCD //
    LCDTextOptions opts = LCDTextOptions_Centered;
    if(activeProgram == NULL) {
        if(autonomousPrograms.nodeCount > 0) opts |= LCDTextOptions_RightArrow;
        LCD_setText(lcd, 2, opts, "(none)");
    } else {
        opts |= LCDTextOptions_LeftArrow;
        if(activeProgram->next != NULL) opts |= LCDTextOptions_RightArrow;
        Command* cmd = activeProgram->data;
        LCD_setText(lcd, 2, opts, Command_getName(cmd));
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

unsigned int Autonomous_addProgram(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    if(Autonomous_hasProgram(cmd)) return 0;
    List_insertLast(&autonomousPrograms, List_newNode(cmd));
    autoChanged = true;
    return autonomousPrograms.nodeCount;
}

bool Autonomous_removeProgram(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    if(!Autonomous_hasProgram(cmd)) return false;
    ListNode* node = List_findNode(&autonomousPrograms, cmd);
    if(node != NULL) {
        if(activeProgram == node) {
            setActiveProgram(NULL);
        }
        List_remove(node);
        free(node);
        return true;
    }
    return false;
}

bool Autonomous_hasProgram(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return (List_findNode(&autonomousPrograms, cmd) != NULL);
}

unsigned int Autonomous_getProgramCount() {
    return autonomousPrograms.nodeCount;
}

// starting at 1 //
Command* Autonomous_getProgramByNumber(unsigned int n) {
    ErrorIf(n < 1 || n > Autonomous_getProgramCount(), VEXOS_ARGRANGE);
    
    ListNode* node = List_getByIndex(&autonomousPrograms, n - 1);
    return (node != NULL)? node->data: NULL;
}

Command* Autonomous_getSelectedProgram() {
    if(activeProgram == NULL) return NULL;
    return (Command*) activeProgram->data;
}

void Autonomous_setSelectedProgram(Command* cmd) {
    if(cmd == NULL) activeProgram = NULL;
    ListNode* active = List_findNode(&autonomousPrograms, cmd);
    if(active == NULL) {
        Autonomous_addProgram(cmd);
        active = autonomousPrograms.lastNode; // the newly added node //
        return;
    }
    setActiveProgram(active);
}

void Autonomous_setSelectedProgramByNumber(unsigned int n) {
    ErrorIf(n > Autonomous_getProgramCount(), VEXOS_ARGRANGE);
    
    Autonomous_setSelectedProgram(Autonomous_getProgramByNumber(n));
}

bool Autonomous_restoreLastProgram() {
    unsigned int value = (unsigned int) GlobalData(GLOBALDATA_AUTO_PROGRAM);
    if(value > 0) {
        Autonomous_setSelectedProgramByNumber(value);
        return true;
    }
    return false;
}

/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

Window* Autonomous_getWindow() {
    static Window* window = NULL;
    if(window) return window;
    window = Window_new("Autonomous Program", &updateWindow);
    Window_setSize(window, 27, 5);
    Window_setPosition(window, 48, 0);
    return window;
}

LCDScreen* Autonomous_getLCDScreen() {
    static LCDScreen* screen = NULL;
    if(screen) return screen;
    screen = LCDScreen_new("Autonomous", &drawLCDScreen, &getLCDStatus);
    return screen;
}

