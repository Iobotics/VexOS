//
//  Error.h
//  VexOS
//
//  Created by Jeff Malins on 12/8/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
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
    VEXOS_OPINVALID
} ErrorCode;

void VexOS_failWithError(ErrorCode);

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
