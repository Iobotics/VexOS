//
//  Interrupt.h
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

#ifndef _INTERRUPT_h
#define _INTERRUPT_h

#include "VexOS.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

#define MAX_INTERRUPT_HANDLERS  10

typedef void (InterruptHandler)(void* object);

bool Interrupt_isEnabled();
void Interrupt_enable();
void Interrupt_disable();
void Interrupt_add(void* object, InterruptHandler* handler, int freq, int order);
void Interrupt_remove(void* object, InterruptHandler* handler);

#endif // _INTERRUPT_h
