//
//  Status.c
//  VexOS for Vex Cortex, User Interface Layer
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

#include "API.h"

#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static void updateWindow(Window* win, bool full) {
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left = innerRect.left;
    unsigned char top  = innerRect.top;
    // loop frequency //
    PrintTextToGD(top, left, Color_Black, "Loop: %4.0f Hz \n", VexOS_getLoopFrequency());
    // run mode //
    if(full) {
        switch(VexOS_getRunMode()) {
            case RunMode_Setup:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Setup\n");
                break;
            case RunMode_Initialize:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Disabled\n");
                break;
            case RunMode_Autonomous:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Auto\n");
                break;
            case RunMode_Operator:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Operator\n");
                break;
        }
    }
    // run time //
    PrintTextToGD(top + 2, left, Color_Black, "Run:  %d s\n", VexOS_getRunTime());
}

static void drawLCDScreen(LCDScreen* screen, LCDButtonType pressed) {
    const char* modeStr = NULL;
    switch(VexOS_getRunMode()) {
        case RunMode_Setup:         modeStr = "Setup";    break;
        case RunMode_Initialize:    modeStr = "Disabled"; break;
        case RunMode_Autonomous:    modeStr = "Auto";     break;
        case RunMode_Operator:      modeStr = "Operator"; break;
    }
    LCD* lcd = LCDScreen_getLCD(screen);
    LCD_setText(lcd, 1, LCDTextOptions_None, "%-8s%6d s", modeStr, VexOS_getRunTime());
    LCD_setText(lcd, 2, LCDTextOptions_None, "<VexOS>%6.0f Hz", VexOS_getLoopFrequency());
}


/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

Window* Status_getWindow() {
    static Window* window = NULL;
    if(window) return window;
    window = Window_new("Robot Status", &updateWindow);
    Window_setSize(window, 16, 3);
    return window;
}

LCDScreen* Status_getLCDScreen() {
    static LCDScreen* screen = NULL;
    if(screen) return screen;
    screen = LCDScreen_new("Robot Status", &drawLCDScreen, NULL);
    return screen;
}

