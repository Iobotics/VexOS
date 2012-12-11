//
//  LCDScreen.c
//  VexOS
//
//  Created by Jeff Malins on 12/7/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "LCDScreen.h"
#include "Error.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

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
