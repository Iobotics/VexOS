//
//  Window.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _Window_h
#define _Window_h

#include "VexOS.h"

/********************************************************************
 * Window Structures                                                *
 ********************************************************************/

typedef struct {
    unsigned char left, top, right, bottom;
} Rect;

typedef struct {
    unsigned char x, y;
} Point;

struct Window {
    String              name;
    char                width;
    char                height;
    WindowDrawCallback* drawCallback;
    // internal state //
    ListNode*           dashboardNode;
    Rect                outerRect;
    Rect                innerRect;
    Point               position;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Window_draw(Window*, bool);

#endif // _Window_h
