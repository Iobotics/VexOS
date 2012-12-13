//
//  DebugValue.c
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

#include "LCD.h"
#include "LCDScreen.h"
#include "Window.h"
#include "Error.h"

/********************************************************************
 * DebugValue Structure                                             *
 ********************************************************************/

struct DebugValue {
    String          name;
    DebugValueType  valueType;
    unsigned long   changeTime;
    unsigned long   displayTime;
    String          formatString;
    String          valueString;
};

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define CHANGE_PERIOD 1000

static ListNode* currentValue;
extern List DebugValues;

static Window* debugWindow;
static LCDScreen* debugLCDScreen;

static void setCurrentValue(ListNode* node) {
    currentValue = node;
    // store in global data //
    GlobalData(GLOBALDATA_DEBUG_VALUE) = (node)? (List_indexOfNode(node) + 1): 0;
}

static bool restoreCurrentValue() {
    unsigned int n = (unsigned int) GlobalData(GLOBALDATA_DEBUG_VALUE);
    if(n > 0) {
        setCurrentValue(List_getByIndex(&DebugValues, n - 1));
        return true;
    }
    return false;
}

static void updateWindow(Window* win, bool full) {
    static unsigned long lastTime;
    static unsigned int  lastCount;
    unsigned char left = win->innerRect.left;
    unsigned char top  = win->innerRect.top;
    
    // if a value was removed, we need to clear and repaint //
    if(DebugValues.nodeCount < lastCount || full) {
        ClearGD(top, left, win->innerRect.bottom, win->innerRect.right, false);
        lastTime = 0; // repaint all //
    }
    
    // get the length of the title //
    size_t length = 0;
    ListNode* node = DebugValues.firstNode;
    while(node != NULL) {
        DebugValue* value = (DebugValue*) node->data;
        size_t nlength = strlen(value->name);
        length = (nlength > length)? nlength: length;
        node = node->next;
    }
    if(length > 12) length = 12;
    
    // print the values //
    unsigned long time = GetMsClock();
    int line = top;
    node = DebugValues.firstNode;
    while(node != NULL) {
        DebugValue* value = node->data;
        // check if value has changed since last update //
        if(value->changeTime > lastTime || time > value->displayTime) {
            Color color = ((time - value->changeTime) < CHANGE_PERIOD)?
        Color_DarkGreen: Color_Black;
            PrintTextToGD(line, left, color, "%-15.15s %-12.12s\n", value->name, value->valueString);
            value->displayTime = (value->displayTime > time)? (time + CHANGE_PERIOD): ULONG_MAX;
        }
        line++;
        node = node->next;
    }
    lastTime  = time;
    lastCount = DebugValues.nodeCount;
}

static bool getLCDStatus(LCDScreen* screen) {
    return DebugValues.nodeCount > 0;
}

static void drawLCDScreen(LCDScreen* screen, LCDButtonType pressed) {
    static bool first = true;
    // restore last value, if needed //
    if(first) {
        restoreCurrentValue();
        first = false;
    }
    
    // try to default first value //
    if(currentValue == NULL) {
        setCurrentValue(DebugValues.firstNode);
    }
    
    // move the selection //
    if(pressed == LCDButtonType_Right) {
        if(currentValue->next != NULL) {
            setCurrentValue(currentValue->next);
        }
    } else if(pressed == LCDButtonType_Left) {
        if(currentValue->prev != NULL) {
            setCurrentValue(currentValue->prev);
        }
    }
    
    // print the value and name to the LCD //
    DebugValue* value = (DebugValue*) currentValue->data;
    LCDTextOptions opts  = LCDTextOptions_Centered;
    LCD_setText(1, opts, value->name);
    if(currentValue->next != NULL) opts |= LCDTextOptions_RightArrow;
    if(currentValue->prev != NULL) opts |= LCDTextOptions_LeftArrow;
    LCD_setText(2, opts, (value->valueString)? value->valueString: "(null)");
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

List DebugValues;

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

DebugValue* DebugValue_new(String name, DebugValueType type) {
    ErrorEntryPoint();
    DebugValue* ret = DebugValue_newWithFormat(name, type, NULL);
    ErrorEntryClear();
    return ret;
}

DebugValue* DebugValue_newWithFormat(String name, DebugValueType type, String format) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(type < DebugValueType_Int || type > DebugValueType_Format, VEXOS_ARGRANGE);
    
    DebugValue* value = malloc(sizeof(DebugValue));
    value->name       = name;
    value->valueType  = type;
    value->changeTime = GetMsClock();
    // set the format string //
    switch(type) {
        case DebugValueType_Int:    value->formatString = "%d"; break;
        case DebugValueType_Bool:
        case DebugValueType_String: value->formatString = "%s"; break;
        case DebugValueType_Float:  value->formatString = "%f"; break;
        case DebugValueType_Format:
            ErrorMsgIf(format == NULL, VEXOS_ARGNULL, "Format string is NULL for DebugValue: %s", name);
            value->formatString = format;
            break;
    }
    value->valueString = NULL;
    List_insertLast(&DebugValues, List_newNode(value));
    return value;
}

DebugValue* DebugValue_delete(DebugValue* value) {
    ErrorIf(value == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&DebugValues, value);
    if(node == NULL) return value;
    if(currentValue == node) setCurrentValue(NULL);
    List_remove(node);
    
    free((void*) value->valueString);
    free(value);
    return NULL;
}

void DebugValue_set(DebugValue* value, ...) {
    ErrorIf(value == NULL, VEXOS_ARGNULL);
    
    // create the new value string from varargs //
    char* vstring;
    va_list argp;
    va_start(argp, value);
    if(value->valueType == DebugValueType_Bool) {
        vstring = va_arg(argp, int)? "true": "false";
    } else {
        vasprintf(&vstring, value->formatString, argp);
    }
    va_end(argp);
    
    // compare the strings //
    if(value->valueString == NULL || strcmp(value->valueString, vstring) != 0) {
        // different: free old string //
        if(value->valueType != DebugValueType_Bool && value->valueString != NULL) {
            free((void*) value->valueString);
        }
        // update if different //
        value->valueString = vstring;
        value->changeTime  = GetMsClock();
    } else {
        // not different, free new string //
        free(vstring);
    }
}

Window* DebugValue_getWindow() {
    if(debugWindow) return debugWindow;
    return (debugWindow = Window_new(27, 15, "Debug Values", &updateWindow));
}

LCDScreen* DebugValue_getLCDScreen() {
    if(debugLCDScreen) return debugLCDScreen;
    return (debugLCDScreen = LCDScreen_new("Debug Values", &drawLCDScreen, &getLCDStatus));
}
