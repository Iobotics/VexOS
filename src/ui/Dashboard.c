//
//  Dashboard.c
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

#include "UserInterface.h"
#include "Window.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct Dashboard {
    String    name;
    bool      refresh;
    List      windowList;
    ListNode* windowNode;
};

static List dashboards;
static unsigned int dashNumber = 0;

static void setDashNumber(unsigned int n) {
    // refresh the newly set dashboard //
    if(n > 0 && dashNumber != n) {
        Dashboard* dash  = List_getDataByIndex(&dashboards, n - 1);
        dash->windowNode = NULL;
        dash->refresh    = true;
    }
    dashNumber = n;
    GlobalData(GLOBALDATA_DASH_NUMBER) = n;
}

static bool getLCDStatus(LCDScreen* screen) {
    return (dashboards.nodeCount > 0);
}

static void drawLCDScreen(LCDScreen* screen, LCDButtonType pressed) {
    LCD* lcd = LCDScreen_getLCD(screen);

    LCD_setText(lcd, 1, LCDTextOptions_Centered, "Dashboard");
    unsigned int count = dashboards.nodeCount;
    if(pressed == LCDButtonType_Right) {
        if(dashNumber < count) setDashNumber(dashNumber + 1);
    } else if(pressed == LCDButtonType_Left) {
        if(dashNumber > 0) setDashNumber(dashNumber - 1);
    }

    LCDTextOptions opts = LCDTextOptions_Centered;
    if(dashNumber < count) opts |= LCDTextOptions_RightArrow;
    if(dashNumber > 0)     opts |= LCDTextOptions_LeftArrow;

    if(dashNumber == 0) {
        LCD_setText(lcd, 2, opts, "(none)");
    } else {
        Dashboard* dash = List_getDataByIndex(&dashboards, dashNumber - 1);
        LCD_setText(lcd, 2, opts, dash->name);
    }
}

static void drawBackground() {
    // paint "background" //
    PrintTextToGD(28, 0, 0x888888, "<VexOS>   v%d.%d.%d\n", 
                  VEXOS_MAJOR_VERSION, VEXOS_MINOR_VERSION, VEXOS_BUILD_VERSION);
    const char* name = VexOS_getProgramName();
    if(name != NULL) {
        PrintTextToGD(28, 40, 0x888888, "%39.39s\n", name);
    }
}

static void eventCallback(EventType type, void* state) {
    static unsigned long nextTime = 0;
    unsigned long time = GetMsClock();
    
    // make sure we display the dashboard //
    if(dashNumber == 0 || time < nextTime) return;

    Dashboard* dash = List_getDataByIndex(&dashboards, dashNumber - 1);
    if(!dash->windowNode) {
        dash->windowNode = dash->windowList.firstNode;
        // at start of list, clear GD //
        if(dash->refresh) ResetGD();
    } else {
        dash->windowNode = dash->windowNode->next;
        // at end of list, clear refresh //
        if(!dash->windowNode && dash->refresh) {
            drawBackground();
            dash->refresh = false;
        }
    }
    if(!dash->windowNode) return;
    Window_draw((Window*) dash->windowNode->data, dash->refresh);

    // set next display time //
    nextTime = time + DASH_PERIOD;
}

static void errorCallback(EventType type, void* state) {
    ResetGD();
    PrintTextToGD(1, 1, Color_Red, Error_getMessage());
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Dashboard* Dashboard_new(String name) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    Dashboard* ret = malloc(sizeof(Dashboard));
    ret->name       = name;
    memset(&ret->windowList, 0, sizeof(List));
    ret->windowNode = NULL;
    List_insertLast(&dashboards, List_newNode(ret));
    
    // check for first LCD and install handler //
    if(dashboards.nodeCount == 1) {
        VexOS_addEventHandler(EventType_DisabledPeriodic,   &eventCallback, NULL);
        VexOS_addEventHandler(EventType_AutonomousPeriodic, &eventCallback, NULL);
        VexOS_addEventHandler(EventType_OperatorPeriodic,   &eventCallback, NULL);
        VexOS_addEventHandler(EventType_SystemError,        &errorCallback, NULL);
    }
    
    return ret;
}

Dashboard* Dashboard_delete(Dashboard* dash) {
    if(dash) {
        ListNode* node = List_findNode(&dashboards, dash);
        if(node) List_remove(node);
        free(node);
    }
    // check for last LCD and remove handler //
    if(dashboards.nodeCount == 0) {
        VexOS_removeEventHandler(EventType_DisabledPeriodic,   &eventCallback);
        VexOS_removeEventHandler(EventType_AutonomousPeriodic, &eventCallback);
        VexOS_removeEventHandler(EventType_OperatorPeriodic,   &eventCallback);
        VexOS_removeEventHandler(EventType_SystemError,        &eventCallback);
    }
    return NULL;
}

void Dashboard_refresh() {
    if(dashNumber > 0) {
        Dashboard* dash = List_getDataByIndex(&dashboards, dashNumber - 1);
        if(!dash) return;
        dash->refresh    = true;
        dash->windowNode = NULL;
    }
}

void Dashboard_addWindow(Dashboard* dash, Window* win) {
    ErrorIf(dash == NULL, VEXOS_ARGNULL);
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(Window_getDashboard(win), VEXOS_OPINVALID, 
               "Window is already on a Dashboard: %s", Window_getName(win));

    List_insertLast(&dash->windowList, List_newNode(win));
    Window_setDashboard(win, dash);
    dash->refresh = true;
}

void Dashboard_addWindowWithPosition(Dashboard* dash, Window* win, char xpos, char ypos) {
    Window_setPosition(win, xpos, ypos);
    Dashboard_addWindow(dash, win);
}

void Dashboard_removeWindow(Dashboard* dash, Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    Dashboard* wdash = Window_getDashboard(win);
    ErrorMsgIf(wdash != dash, VEXOS_OPINVALID, 
               "Window is not part of a Dashboard: %s", Window_getName(win));

    ListNode* node = List_findNode(&dash->windowList, win);
    if(node) {
        Window_setDashboard(win, NULL);
        List_remove(node);
    }
}

const List* Dashboard_getList() {
    return &dashboards;
}

bool Dashboard_restoreLast() {
    unsigned int n = (unsigned int) GlobalData(GLOBALDATA_DASH_NUMBER);
    if(n > 0 && n <= dashboards.nodeCount) {
        setDashNumber(n);
        return true;
    }
    setDashNumber(0);
    return false;
}


/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

LCDScreen* Dashboard_getLCDScreen() {
    static LCDScreen* screen = NULL;
    if(screen) return screen;
    screen = LCDScreen_new("Dashboard", &drawLCDScreen, &getLCDStatus);
    return screen;
}

