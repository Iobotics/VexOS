//
//  Dashboard.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Dashboard.h"
#include "Window.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static bool dashboardEnabled;
static bool fullRefresh;
static ListNode* lastWindowNode;

void drawBackground() {
    // paint "background" //
    PrintTextToGD(28, 0, 0x888888, "<VexOS>   v0.9.0\n");
    const char* name = VexOS_getProgramName();
    if(name != NULL) {
        PrintTextToGD(28, 40, 0x888888, "%39.39s\n", name);
    }
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

List DashboardWindows;

void Dashboard_update() {
    if(!dashboardEnabled) return;
    // get next window to display //
    if(lastWindowNode == NULL) {
        lastWindowNode = DashboardWindows.firstNode;
    } else {
        lastWindowNode = lastWindowNode->next;
        if(lastWindowNode == NULL && fullRefresh) {
            drawBackground();
            fullRefresh = false;
        }
    }
    if(lastWindowNode == NULL) return;
    Window_draw((Window*) lastWindowNode->data, fullRefresh);
}

bool Dashboard_restoreEnabled() {
    unsigned long value = GlobalData(GLOBALDATA_DASH_ENABLED);
    if(value > 0) {
        Dashboard_setEnabled(value - 1);
        return true;
    }
    return false;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

bool Dashboard_isEnabled() {
    return dashboardEnabled;
}

void Dashboard_setEnabled(bool enabled) {
    if(!dashboardEnabled) Dashboard_refresh();
    dashboardEnabled = enabled;
    // store in global data //
    GlobalData(GLOBALDATA_DASH_ENABLED) = (enabled + 1);
}

void Dashboard_refresh() {
    ResetGD();
    fullRefresh    = true;
    lastWindowNode = NULL;
}

void Dashboard_addWindow(Window* win, unsigned char xpos, unsigned char ypos) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    if(!Window_move(win, xpos, ypos)) return;
    if(win->dashboardNode->list == NULL) {
        List_insertLast(&DashboardWindows, win->dashboardNode);
    };
    Dashboard_refresh();
}

void Dashboard_removeWindow(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    List_remove(win->dashboardNode);
    Dashboard_refresh();
}
