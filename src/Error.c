//
//  Error.c
//  VexOS
//
//  Created by Jeff Malins on 12/9/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Error.h"
#include "LCD.h"

jmp_buf ErrorEvn;
String  ErrorFunction;
String  ErrorMessage;

static String getMessage(ErrorCode error) {
    switch(error) {
        case VEXOS_ARGNULL:     return "Required argument is null";
        case VEXOS_ARGRANGE:    return "Argument is out of range";
        case VEXOS_ARGINVALID:  return "Argument is invalid";
        case VEXOS_OPINVALID:   return "Invalid operation";
        default:
            return "No error"; // this should not occur //
    }
}

void VexOS_failWithError(ErrorCode error) {
    char* msg;
    if(ErrorFunction) {
        asprintf(&msg, "%s: %s (%s)\n", ErrorFunction, getMessage(error), ErrorMessage);
    } else if(ErrorMessage) {
        asprintf(&msg, "ERROR: %s (%s)", getMessage(error), ErrorMessage);
    } else {
        asprintf(&msg, "ERROR: %s", getMessage(error));
    }
    // print to terminal //
    Info(msg);
    // if LCD is enabled, display the error //
    if(LCD_isEnabled()) {
        LCD_setText(1, LCDTextOptions_Centered, "ERROR");
        LCD_setText(2, LCDTextOptions_None, msg);
    }
    // if Dashboard is enabled, display the error //
    if(Dashboard_isEnabled()) {
        ResetGD();
        PrintFrameToGD(1, 1, 29, 78, Color_DarkRed);
        PrintTextToGD(2, 3, Color_DarkRed, msg);
    }
    while(true) Wait(1000);
}
