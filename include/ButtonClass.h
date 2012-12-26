//
//  ButtonClass.h
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

#define DefineButtonClass(xclass, ...) \
    static void constructor(va_list); \
    static bool get(); \
    static Button* self; \
    typedef struct Fields __VA_ARGS__ Fields; \
    ButtonClass xclass = { \
        .name        = #xclass, \
        .selfPtr     = &self, \
        .fieldSize   = sizeof(struct Fields), \
        .constructor = &constructor, \
        .get         = &get \
    }; \
    void Button_setvName(Button*, String, va_list); \
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
