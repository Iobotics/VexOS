//
//  Error.h
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/08/2012.
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

#ifndef _Error_h
#define _Error_h

#include <setjmp.h>
#include "VexOS.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

extern jmp_buf ErrorEvn;
extern String  ErrorFunction;
extern String  ErrorMessage;

// error enumeration //
typedef enum {
    VEXOS_NOERROR = 0,
    VEXOS_ARGNULL,
    VEXOS_ARGRANGE,
    VEXOS_ARGINVALID,
    VEXOS_OPINVALID,
    VEXOS_HARDWARELOCK,
    VEXOS_COMMANDLOCK,
    VEXOS_BUTTONLOCK,
    VEXOS_OBJTYPE
} ErrorCode;

void   Error_setCode(ErrorCode);
String Error_getMessage();

#ifdef DEBUG
  /********************************************************************
   * Debugging Error Macros (includes trace information)              *
   ********************************************************************/

  #define ErrorIf(test, errorCode) \
      if(test) { \
          if(!ErrorFunction) ErrorFunction = __FUNCTION__; \
          ErrorMessage = #test; \
          longjmp(ErrorEvn, errorCode); \
      }

  #define ErrorMsgIf(test, errorCode, msg, ...) \
      if(test) { \
          if(!ErrorFunction) ErrorFunction = __FUNCTION__; \
          asprintf((char **) &ErrorMessage, msg, ##__VA_ARGS__); \
          longjmp(ErrorEvn, errorCode); \
      }

  #define RaiseError(errorCode, msg, ...) \
      if(!ErrorFunction) ErrorFunction = __FUNCTION__; \
      asprintf((char **) &ErrorMessage, msg, ##__VA_ARGS__); \
      longjmp(ErrorEvn, errorCode);

  #define ErrorEntryPoint() \
      ErrorFunction = __FUNCTION__;

  #define ErrorEntryClear() \
      ErrorFunction = NULL;

#else 
  /********************************************************************
   * Production Error Macros (removes trace to save space)            *
   ********************************************************************/

  #define ErrorIf(test, errorCode) \
      if(test) longjmp(ErrorEvn, errorCode);

  #define ErrorMsgIf(test, errorCode, msg, ...) \
      if(test) longjmp(ErrorEvn, errorCode);

  #define RaiseError(errorCode, msg, ...) \
      longjmp(ErrorEvn, errorCode);

  #define ErrorEntryPoint()
  #define ErrorEntryClear()

  #define Debug(msg, ...)
  #define Info(msg, ...)  PrintToScreen(msg, ##__VA_ARGS__)

#endif

/********************************************************************
 * Deep Debugging Macros (used rarely)                              *
 ********************************************************************/
#ifdef DEEP_DEBUG
  #define Debug(msg, ...) PrintToScreen(msg "\n", ##__VA_ARGS__)
#else 
  #define Debug(msg, ...)
#endif

#define Info(msg, ...)  PrintToScreen(msg, ##__VA_ARGS__)

#endif // _Error_h
