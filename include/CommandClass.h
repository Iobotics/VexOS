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

#define DefineCommandClass(xclass, ...) \
    static void constructor(va_list argp); \
    static void initialize(); \
    static void execute(); \
    static bool isFinished(); \
    static void end(); \
    static void interrupted(); \
    static Command* self; \
    typedef struct Fields __VA_ARGS__ Fields; \
    CommandClass xclass = { \
        .name        = #xclass, \
        .selfPtr     = &self, \
        .fieldSize   = sizeof(struct Fields), \
        .constructor = &constructor, \
        .initialize  = &initialize, \
        .execute     = &execute, \
        .isFinished  = &isFinished, \
        .end         = &end, \
        .interrupted = &interrupted \
    }; \
    void Command_setvName(Command* cmd, String fmtString, va_list argp); \
    static void setName(String fmtString, ...) { \
        va_list argp; \
        va_start(argp, fmtString); \
        Command_setvName(self, fmtString, argp); \
        va_end(argp); \
    } \
    void Command_setvArgs(Command* cmd, String fmtString, va_list argp); \
    static void setArgs(String fmtString, ...) { \
        va_list argp; \
        va_start(argp, fmtString); \
        Command_setvArgs(self, fmtString, argp); \
        va_end(argp); \
    } \
    void Command_require(Command* cmd, Subsystem* sys); \
    static void require(Subsystem* sys) { \
        Command_require(self, sys); \
    } \
    void Command_setTimeout(Command* cmd, unsigned long time); \
    static void setTimeout(unsigned long time) { \
        Command_setTimeout(self, time); \
    } \
    bool Command_isTimedOut(Command* cmd); \
    static bool isTimedOut() { \
        return Command_isTimedOut(self); \
    } \
    void Command_setInterruptible(Command* cmd, bool value); \
    static void setInterruptible(bool value) { \
        Command_setInterruptible(self, value); \
    } \
    void Command_checkInstance(Command* cmd, CommandClass* class); \
    static void checkInstance(Command* cmd) { \
        Command_checkInstance(cmd, &xclass); \
    }

#define DefineCommandGroup(xclass) \
    static void constructor(va_list argp); \
    static Command* self; \
    CommandClass xclass = { \
        .name             = #xclass, \
        .groupConstructor = &constructor, \
        .groupSelfPtr     = &self \
    }; \
    void Command_setvName(Command* cmd, String fmtString, va_list argp); \
    static void setName(String fmtString, ...) { \
        va_list argp; \
        va_start(argp, fmtString); \
        Command_setvName(self, fmtString, argp); \
        va_end(argp); \
    } \
    void Command_setvArgs(Command* cmd, String fmtString, va_list argp); \
    static void setArgs(String fmtString, ...) { \
        va_list argp; \
        va_start(argp, fmtString); \
        Command_setvArgs(self, fmtString, argp); \
        va_end(argp); \
    } \
    void Command_setInterruptible(Command* cmd, bool value); \
    static void setInterruptible(bool value) { \
        Command_setInterruptible(self, value); \
    } \
    void Command_checkInstance(Command* cmd, CommandClass* class); \
    static void checkInstance(Command* cmd) { \
        Command_checkInstance(cmd, &xclass); \
    }

/********************************************************************
 * GCC Compiler Pragmas                                             *
 ********************************************************************/

#pragma GCC diagnostic ignored "-Wunused-function"

#endif // _CommandClass_h
