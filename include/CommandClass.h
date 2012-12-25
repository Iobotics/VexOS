//
//  CommandClass.h
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

#ifndef _CommandClass_h
#define _CommandClass_h

#include "VexOS.h"

/********************************************************************
 * CommandClass Structures                                          *
 ********************************************************************/

struct CommandClass {
    bool         initialized;
    String const name;
    Command**    selfPtr;
    size_t       fieldSize;
    unsigned int lastInstanceId;
    // Command methods //
    void (*constructor)(va_list);
    void (*destructor)();
    void (*initialize)();
    void (*execute)();
    bool (*isFinished)();
    void (*end)();
    void (*interrupted)();
    // CommandGroup extension members //
    void (*groupConstructor)(va_list);
    Command** groupSelfPtr;
};

/********************************************************************
 * Command Structure (public version)                               *
 ********************************************************************/

#ifndef _Command_h
struct Command {
    struct Fields*      fields;
    const CommandClass* class;
};
#endif

/********************************************************************
 * User-friendliness Inheritance Macros                             *
 ********************************************************************/

#define DefineCommandClass(class, ...) \
    static void constructor(va_list); \
    static void initialize(); \
    static void execute(); \
    static bool isFinished(); \
    static void end(); \
    static void interrupted(); \
    static Command* self; \
    typedef struct Fields __VA_ARGS__ Fields; \
    CommandClass class = { \
        .name        = #class, \
        .selfPtr     = &self, \
        .fieldSize   = sizeof(struct Fields), \
        .constructor = &constructor, \
        .initialize  = &initialize, \
        .execute     = &execute, \
        .isFinished  = &isFinished, \
        .end         = &end, \
        .interrupted = &interrupted \
    }; \
    void Command_setvName(Command*, String, va_list); \
    static void setName(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Command_setvName(self, fmt, argp); \
        va_end(argp); \
    } \
    void Command_setvArgs(Command*, String, va_list); \
    static void setArgs(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Command_setvArgs(self, fmt, argp); \
        va_end(argp); \
    } \
    void Command_require(Command*, Subsystem*); \
    static void require(Subsystem* sys) { \
        Command_require(self, sys); \
    } \
    void Command_setTimeout(Command*, unsigned long time); \
    static void setTimeout(unsigned long time) { \
        Command_setTimeout(self, time); \
    } \
    bool Command_isTimedOut(Command*); \
    static bool isTimedOut() { \
        return Command_isTimedOut(self); \
    } \
    void Command_setInterruptible(Command*, bool); \
    static void setInterruptible(bool value) { \
        Command_setInterruptible(self, value); \
    }

#define DefineCommandGroup(class) \
    static void constructor(va_list); \
    static Command* self; \
    CommandClass class = { \
        .name             = #class, \
        .groupConstructor = &constructor, \
        .groupSelfPtr     = &self \
    }; \
    void Command_setvName(Command*, String, va_list); \
    static void setName(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Command_setvName(self, fmt, argp); \
        va_end(argp); \
    } \
    void Command_setvArgs(Command*, String, va_list); \
    static void setArgs(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Command_setvArgs(self, fmt, argp); \
        va_end(argp); \
    } \
    void Command_setInterruptible(Command*, bool); \
    static void setInterruptible(bool value) { \
        Command_setInterruptible(self, value); \
    }

/********************************************************************
 * GCC Compiler Pragmas                                             *
 ********************************************************************/

#pragma GCC diagnostic ignored "-Wunused-function"

#endif // _CommandClass_h
