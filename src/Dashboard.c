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
