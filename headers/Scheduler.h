//
//  Scheduler.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _Scheduler_h
#define _Scheduler_h

#include "VexOS.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

typedef struct ButtonScheduler ButtonScheduler;

void Scheduler_run();
void Scheduler_add(Command*);
void Scheduler_addButtonScheduler(ButtonScheduler*);
void Scheduler_windowUpdate(Window*, bool);

#endif // _Scheduler_h
