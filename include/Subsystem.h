//
//  Subsystem.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
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
    void (*constructor)(va_list);
    void (*initDefaultCommand)();
    // internal fields //
    Command* currentCommand;
    Command* defaultCommand;
    bool     initializedDefaultCommand;
    bool     isEnabled;
};

/********************************************************************
 * Protected API: Subsystem                                         *
 ********************************************************************/

extern List Subsystems;

Command* Subsystem_getDefaultCommand(Subsystem*);
void Subsystem_setDefaultCommand(Subsystem*, Command*);

/********************************************************************
 * User-friendliness Singleton Definition Macros                    *
 ********************************************************************/

#define DeclareSubsystem(class) \
    static void constructor(va_list); \
    static void initDefaultCommand(); \
    static Subsystem* self; \
    Subsystem class = { \
        .name               = #class, \
        .selfPtr            = &self, \
        .constructor        = &constructor, \
        .initDefaultCommand = &initDefaultCommand \
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
