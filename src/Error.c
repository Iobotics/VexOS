//
//  Error.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/09/2012.
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
