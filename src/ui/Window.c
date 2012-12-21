//
//  Window.c
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

#include "API.h"

#include "UserInterface.h"
#include "Window.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct Window {
    String              name;
    char                width;
    char                height;
    WindowDrawCallback* drawCallback;
    Dashboard*          dashboard;
    // internal state //
    bool                valid;
    Rect                outerRect;
    Rect                innerRect;
    Point               position;
};

void computeExtents(Window* win) {
    if(win->width == -1 || win->height == -1) return;
    if(win->position.x == -1 || win->position.y == -1) return;

    char oright  = (win->position.x + win->width + 4);
    char obottom = (win->position.y + win->height + 3);
    ErrorMsgIf(oright > 79 || obottom > 29, VEXOS_OPINVALID, 
               "Window would be off screen: %s", win->name);
    win->outerRect.left   = win->position.x;
    win->outerRect.right  = oright;
    win->outerRect.top    = win->position.y;
    win->outerRect.bottom = obottom;
    win->innerRect.left   = win->position.x + 2;
    win->innerRect.right  = (win->position.x + win->width + 2);
    win->innerRect.top    = win->position.y + 3;
    win->innerRect.bottom = (win->position.y + win->height + 2);
    win->valid = true;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

#define COLOR_BORDER    0xAAAAAA
#define COLOR_TITLE     0x888888

void Window_setDashboard(Window* win, Dashboard* dash) {
    win->dashboard = dash;
}

void Window_draw(Window* win, bool full) {
    if(full) {
        // clear the area //
        ClearGD(win->outerRect.top, win->outerRect.left, win->outerRect.bottom,
                win->outerRect.right, true);  // clear frames //
        ClearGD(win->outerRect.top, win->outerRect.left, win->outerRect.bottom,
                win->outerRect.right, false); // clear text //
        // draw frame //
        PrintFrameToGD(win->outerRect.top + 2, win->outerRect.left, win->outerRect.bottom,
                       win->outerRect.right, COLOR_BORDER);
        PrintFrameToGD(win->outerRect.top, win->outerRect.left, win->outerRect.top + 2,
                       win->outerRect.left + strlen(win->name) + 3, COLOR_BORDER);
        PrintTextToGD(win->outerRect.top + 1, win->outerRect.left + 2, COLOR_TITLE, "%s\n",
                      win->name);
    }
    if(win->drawCallback) {
        win->drawCallback(win, full);
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Window* Window_new(const char* name, WindowDrawCallback* draw) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(draw == NULL, VEXOS_ARGNULL);
    
    Window* win = malloc(sizeof(Window));
    if(win == NULL) return NULL;
    win->name         = name;
    win->width        = -1;
    win->height       = -1;
    win->name         = name;
    win->drawCallback = draw;
    win->dashboard    = NULL;
    // initialize the rectangles //
    win->valid = false;
    memset(&win->position,  -1, sizeof(Point));
    memset(&win->innerRect, -1, sizeof(Rect));
    memset(&win->outerRect, -1, sizeof(Rect));
    return win;
}

Window* Window_delete(Window* win) {
    if(win) {
        if(win->dashboard) Dashboard_removeWindow(win->dashboard, win);
        free(win);
    }
    return NULL;
}

String Window_getName(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->name;
}

Dashboard* Window_getDashboard(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->dashboard;
}

void Window_setSize(Window* win, char width, char height) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorIf(width < 0 || width > 78, VEXOS_ARGRANGE);
    ErrorIf(height < 0 || height > 27, VEXOS_ARGRANGE);

    win->width  = width;
    win->height = height;
    computeExtents(win);
}

char Window_getWidth(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);
    
    return win->width;
}

char Window_getHeight(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);

    return win->height;
}

void Window_setPosition(Window* win, char xpos, char ypos) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorIf(xpos < 0 || xpos > 79, VEXOS_ARGRANGE);
    ErrorIf(ypos < 0 || ypos > 30, VEXOS_ARGRANGE);
    
    win->position.x = xpos;
    win->position.y = ypos;
    computeExtents(win);
}

Point Window_getPosition(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->position;
}

char Window_getLeft(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);
    
    return win->innerRect.left;
}

char Window_getTop(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);

    return win->innerRect.top;
}

Rect Window_getInnerRect(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);

    return win->innerRect;
}

Rect Window_getOuterRect(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(!win->valid, VEXOS_OPINVALID, "Window has incomplete dimensions: %s", win->name);

    return win->outerRect;
}
