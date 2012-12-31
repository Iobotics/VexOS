//
//  Subsystem.h
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

#ifndef _Subsystem_h
#define _Subsystem_h

#include "VexOS.h"

/********************************************************************
 * Subsystem Structure                                              *
 ********************************************************************/

struct Subsystem {
    bool            initialized;
    String const    name;
    Subsystem**     selfPtr;
    size_t          fieldSize;
    struct Fields*  fields;
    // Subsystem methods //
    void (*constructor)();
    void (*initialize)();
    // internal fields //
    Command* currentCommand;
    Command* defaultCommand;
    bool     isEnabled;
};

/********************************************************************
 * Protected API: Subsystem                                         *
 ********************************************************************/

extern List Subsystems;

void Subsystem_setDefaultCommand(Subsystem* sys, Command* cmd);

/********************************************************************
 * User-friendliness Singleton Definition Macros                    *
 ********************************************************************/

#define DefineSubsystem(xclass) \
    static void constructor(); \
    static void initialize(); \
    static Subsystem* self; \
    Subsystem xclass = { \
        .name        = #xclass, \
        .selfPtr     = &self, \
        .constructor = &constructor, \
        .initialize  = &initialize \
    }; \
    static Command* getDefaultCommand() { \
        return Subsystem_getDefaultCommand(self); \
    } \
    static void setDefaultCommand(Command* cmd) { \
        Subsystem_setDefaultCommand(self, cmd); \
    }

/********************************************************************
 * GCC Compiler Pragmas                                             *
 ********************************************************************/

#pragma GCC diagnostic ignored "-Wunused-function"

#endif // _Subsystem_h
