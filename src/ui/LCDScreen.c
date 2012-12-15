//
//  LCDScreen.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/07/2012.
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

#include "LCDScreen.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct LCDScreen {
    String             name;
    LCD*               parent;
    LCDStatusCallback* statusCallback;
    LCDDrawCallback*   drawCallback;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void LCDScreen_setLCD(LCDScreen* screen, LCD* lcd) {
    screen->parent = lcd;
}

bool LCDScreen_isDisplayed(LCDScreen* screen) {
    if(!screen->drawCallback) return false;
    return !screen->statusCallback || screen->statusCallback(screen);
}

void LCDScreen_draw(LCDScreen* screen, LCDButtonType pressed) {
    if(screen->drawCallback) screen->drawCallback(screen, pressed);
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

LCDScreen* LCDScreen_new(String name, LCDDrawCallback* draw, LCDStatusCallback* status) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(draw == NULL, VEXOS_ARGNULL);
    
    LCDScreen* screen = malloc(sizeof(LCDScreen));
    screen->name           = name;
    screen->drawCallback   = draw;
    screen->statusCallback = status;
    return screen;
}

LCDScreen* LCDScreen_delete(LCDScreen* screen) {
    free(screen);
    return NULL;
}

String LCDScreen_getName(LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);

    return screen->name;
}

LCD* LCDScreen_getLCD(LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);

    return screen->parent;
}

