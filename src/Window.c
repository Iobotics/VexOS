//
//  Window.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Window.h"
#include "Error.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

#define COLOR_BORDER    0xAAAAAA
#define COLOR_TITLE     0x888888

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

Window* Window_new(unsigned char width, unsigned char height, const char* name, WindowDrawCallback* draw) {
    ErrorIf(width < 0 || width > 78, VEXOS_ARGRANGE);
    ErrorIf(height < 0 || height > 27, VEXOS_ARGRANGE);
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(draw == NULL, VEXOS_ARGNULL);
    
    Window* win = malloc(sizeof(Window));
    if(win == NULL) return NULL;
    win->name           = name;
    win->width          = width;
    win->height         = height;
    win->name           = name;
    win->drawCallback   = draw;
    win->dashboardNode  = List_newNode(win);
    // initialize the rectangles //
    win->position.x       = -1;
    win->position.y       = -1;
    win->outerRect.left   = -1;
    win->outerRect.top    = -1;
    win->outerRect.right  = -1;
    win->outerRect.bottom = -1;
    win->innerRect.left   = -1;
    win->innerRect.top    = -1;
    win->innerRect.right  = -1;
    win->innerRect.bottom = -1;
    return win;
}

char Window_GetWidth(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->width;
}

char Window_Height(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->height;
}

Point Window_GetPosition(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->position;
}

String Window_getTitle(Window* win) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    
    return win->name;
}

bool Window_move(Window* win, unsigned char xpos, unsigned char ypos) {
    ErrorIf(win == NULL, VEXOS_ARGNULL);
    ErrorIf(xpos < 0 || xpos > 79, VEXOS_ARGRANGE);
    ErrorIf(ypos < 0 || ypos > 30, VEXOS_ARGRANGE);
    
    // check for position overflow //
    if(xpos < 0 || xpos > 79 || ypos < 0 || ypos > 29) return false;
    // check for extent overflow //
    char oright  = (xpos + win->width + 4);
    char obottom = (ypos + win->height + 3);
    if(oright > 79 || obottom > 29) return false;
    win->position.x       = xpos;
    win->position.y       = ypos;
    win->outerRect.left   = xpos;
    win->outerRect.right  = oright;
    win->outerRect.top    = ypos;
    win->outerRect.bottom = obottom;
    win->innerRect.left   = xpos + 2;
    win->innerRect.right  = (xpos + win->width + 2);
    win->innerRect.top    = ypos + 3;
    win->innerRect.bottom = (ypos + win->height + 2);
    return true;
}
