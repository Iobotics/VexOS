//
//  Button.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _Button_h
#define _Button_h

#include "Scheduler.h"

/********************************************************************
 * Button Structure                                                 *
 ********************************************************************/

struct Button {
    // public members //
    struct Fields*     fields;
    const ButtonClass* class;
    // hidden infrastructure fields //
    unsigned int       objectId;
    unsigned int       instanceId;
    String             name;
    bool               toggleState;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Button_executeScheduler(ButtonScheduler*);

void Button_setName(Button*, String, ...);
void Button_setArgs(Button*, String, ...);

#endif // _Button_h
