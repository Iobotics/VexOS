//
//  CommandClass.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
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

#define DeclareCommandClass(class, ...) \
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

/********************************************************************
 * GCC Compiler Pragmas                                             *
 ********************************************************************/

#pragma GCC diagnostic ignored "-Wunused-function"

#endif // _CommandClass_h
