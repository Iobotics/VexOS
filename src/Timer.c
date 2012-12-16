//
//  Timer.c
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

#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static Timer* timers[TIMER_COUNT];

struct Timer { 
    TimerId id;
    String  name;
    bool    enabled;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Timer* Timer_new(String name, TimerId timerId) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(timerId < 1 || timerId > TIMER_COUNT, VEXOS_ARGRANGE);
    ErrorMsgIf(timers[timerId], VEXOS_ARGINVALID, "Timer is already defined: %d", timerId);

    Timer* timer = malloc(sizeof(Timer));
    timer->id      = timerId;
    timer->name    = name;
    timer->enabled = false;

    timers[timerId] = timer;
    return timer;
}

Timer* Timer_delete(Timer* timer) {
    if(timer) {
        if(timer->enabled) {
            StopTimer(timer->id);
        }
        free(timer);
    }
    return NULL;
}

String Timer_getName(Timer* timer) {
    ErrorIf(timer == NULL, VEXOS_ARGNULL);

    return timer->name;
}

bool Timer_isEnabled(Timer* timer) {
    ErrorIf(timer == NULL, VEXOS_ARGNULL);

    return timer->enabled;
}

void Timer_setEnabled(Timer* timer, bool value) {
    ErrorIf(timer == NULL, VEXOS_ARGNULL);

    if(value) {
        StartTimer(timer->id);
    } else {
        StopTimer(timer->id);
    }
    timer->enabled = value;
}

void Timer_preset(Timer* timer, unsigned long value) {
    ErrorIf(timer == NULL, VEXOS_ARGNULL);
    
    PresetTimer(timer->id, value);
}

unsigned long Timer_get(Timer* timer) {
    ErrorIf(timer == NULL, VEXOS_ARGNULL);

    return GetTimer(timer->id);
}
