//
//  LCDScreen.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _LCDScreen_h
#define _LCDScreen_h

#include "VexOS.h"

/********************************************************************
 * LCDScreen Structure                                              *
 ********************************************************************/

struct LCDScreen {
    String             name;
    LCDStatusCallback* statusCallback;
    LCDDrawCallback*   drawCallback;
};

bool LCDScreen_isDisplayed(LCDScreen*);
void LCDScreen_draw(LCDScreen*, LCDButtonType);

#endif // _LCDScreen_h
