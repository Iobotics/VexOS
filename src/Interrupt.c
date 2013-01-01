//
//  PID.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/31/2012.
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

#include "Interrupt.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static bool enabled = false;
static bool locked  = true;
typedef struct {
    void*             object;
    InterruptHandler* handler;
    int               freq;
    int               order;
} InterruptData;

static int numHandlers = 0;
static InterruptData handlers[MAX_INTERRUPT_HANDLERS];

static void runISR() {
    static int count = 0;
    for(int i = 0; i < numHandlers; i++) {
        if(handlers[i].freq % count) continue;
        handlers[i].handler(handlers[i].object);
    }
    count++;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

// hidden method, not in header //
void Interrupt_setLocked(bool value) {
    locked = value;
}

bool Interrupt_isEnabled() {
    return enabled;
}

void Interrupt_enable() {
    if(locked || enabled) return;
    RegisterImeInterruptServiceRoutine(&runISR);
    enabled = true;
}

void Interrupt_disable() {
    if(!enabled) return;
    UnRegisterImeInterruptServiceRoutine(&runISR);
    enabled = false;
}

void Interrupt_add(void* object, InterruptHandler* handler, int freq, int order) {
    ErrorIf(numHandlers == MAX_INTERRUPT_HANDLERS, VEXOS_OPINVALID);

    bool lastEnabled = enabled;
    Interrupt_disable();

    int slot = numHandlers;
    for(int i = 0; i < numHandlers; i++) {
        // are we higher priority than what is in this slot //
        if(slot == numHandlers && order < handlers[i].order) {
            slot = i;
        }
        if(slot != numHandlers) {
            int choice = numHandlers - 1 - (i - slot);
            handlers[choice + 1] = handlers[choice]; 
        } 
    }
    numHandlers++;
    handlers[slot] = (InterruptData) { object, handler, freq, order };

    if(lastEnabled) Interrupt_enable();
}

void Interrupt_remove(void* object, InterruptHandler* handler) {
    bool lastEnabled = enabled;
    Interrupt_disable();

    // look for the handler and copy everything up one slot after it //
    bool found = false;
    for(int i = 0; i < numHandlers; i++) {
        if(!found) {
            found = (handlers[i].object == object && handlers[i].handler == handler);
        }
        if(found && i < numHandlers - 1) {
            handlers[i] = handlers[i + 1];
        }
    }
    if(found) numHandlers--;

    if(lastEnabled) Interrupt_enable();
}

