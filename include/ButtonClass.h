//
//  ButtonClass.h
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _ButtonClass_h
#define _ButtonClass_h

#include "VexOS.h"

/********************************************************************
 * ButtonClass Structures                                           *
 ********************************************************************/

struct ButtonClass {
    bool         initialized;
    String const name;
    Button**     selfPtr;
    size_t       fieldSize;
    unsigned int lastInstanceId;
    // Button methods //
    void (*constructor)(va_list);
    void (*destructor)();
    bool (*get)();
};

/********************************************************************
 * Button Structure (public version)                                *
 ********************************************************************/

#ifndef _Button_h
struct Button {
    struct Fields*     fields;
    const ButtonClass* class;
};
#endif

/********************************************************************
 * User-friendliness Inheritance Macros                             *
 ********************************************************************/

#define DeclareButtonClass(class, ...) \
    static void constructor(va_list); \
    static bool get(); \
    static Button* self; \
    typedef struct Fields __VA_ARGS__ Fields; \
    ButtonClass class = { \
        .name        = #class, \
        .selfPtr     = &self, \
        .fieldSize   = sizeof(struct Fields), \
        .constructor = &constructor, \
        .get         = &get \
    }; \
    void Button_setvName(Button*, String, ...); \
    static void setName(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Button_setvName(self, fmt, argp); \
        va_end(argp); \
    } \
    void Button_setvArgs(Button*, String, va_list); \
    static void setArgs(String fmt, ...) { \
        va_list argp; \
        va_start(argp, fmt); \
        Button_setvName(self, fmt, argp); \
        va_end(argp); \
    }

/********************************************************************
 * GCC Compiler Pragmas                                             *
 ********************************************************************/

#pragma GCC diagnostic ignored "-Wunused-function"

#endif // _ButtonClass_h
