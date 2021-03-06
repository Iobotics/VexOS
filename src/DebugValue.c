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

#include "API.h"

#include "Hardware.h"
#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * DebugValue Structure                                             *
 ********************************************************************/

typedef enum {
    ValueAccessType_Manual,
    ValueAccessType_Pointer,
    ValueAccessType_Callback
} ValueAccessType;

struct DebugValue {
    String              name;
    DebugValueType      valueType;
    String              formatString;
    unsigned long       changeTime;
    unsigned long       displayTime;
    String              valueString;
    // value pull //
    ValueAccessType     accessType;
    void*               valuePtr;
    DebugValueCallback* callback;
};

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define SAMPLE_TIME   250
#define CHANGE_PERIOD 1000

static ListNode* currentValue;
static List debugValues;
static unsigned long nextSampleTime;

static void setCurrentValue(ListNode* node) {
    currentValue = node;
    // store in global data //
    GlobalData(GLOBALDATA_DEBUG_VALUE) = (node)? (List_indexOfNode(node) + 1): 0;
}

static bool restoreCurrentValue() {
    unsigned int n = (unsigned int) GlobalData(GLOBALDATA_DEBUG_VALUE);
    if(n > 0) {
        setCurrentValue(List_getNodeByIndex(&debugValues, n - 1));
        return true;
    }
    return false;
}

static void pullValues(EventType type, void* state) {
    // enforce sampling interval //
    unsigned long time = GetMsClock();
    if(time < nextSampleTime) return;

    ListNode* node = debugValues.firstNode;
    while(node != NULL) {
        DebugValue* value = node->data;
        switch(value->accessType) {
            case ValueAccessType_Manual: break;
            case ValueAccessType_Pointer:
                // if a pointer, deference by type //
                if(!value->valuePtr) break;
                switch(value->valueType) {
                    case DebugValueType_Int:
                        DebugValue_set(value, *((int*) value->valuePtr));
                        break;
                    case DebugValueType_String:
                        DebugValue_set(value, *((String) value->valuePtr));
                        break;
                    case DebugValueType_Float:
                        DebugValue_set(value, *((float*) value->valuePtr));
                        break;
                    case DebugValueType_Bool:
                        DebugValue_set(value, *((bool*) value->valuePtr));
                        break;
                    case DebugValueType_Format: 
                        // this won't happen //
                        break;
                }
                break;
            case ValueAccessType_Callback:
                if(value->callback) value->callback(value);
                break;
        }
        node = node->next;
    }
    nextSampleTime = time + SAMPLE_TIME;
}

static void updateWindow(Window* win, bool full) {
    static unsigned long lastTime;
    static unsigned int  lastCount;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left = innerRect.left;
    unsigned char top  = innerRect.top;
    
    // if a value was removed, we need to clear and repaint //
    if(debugValues.nodeCount < lastCount || full) {
        ClearGD(top, left, innerRect.bottom, innerRect.right, false);
        lastTime = 0; // repaint all //
    }
    
    // get the length of the title //
    size_t length = 0;
    ListNode* node = debugValues.firstNode;
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
    node = debugValues.firstNode;
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
    lastCount = debugValues.nodeCount;
}

static bool getLCDStatus(LCDScreen* screen) {
    return debugValues.nodeCount > 0;
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
        setCurrentValue(debugValues.firstNode);
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
    LCD* lcd = LCDScreen_getLCD(screen);

    DebugValue* value = (DebugValue*) currentValue->data;
    LCDTextOptions opts  = LCDTextOptions_Centered;
    LCD_setText(lcd, 1, opts, value->name);
    if(currentValue->next != NULL) opts |= LCDTextOptions_RightArrow;
    if(currentValue->prev != NULL) opts |= LCDTextOptions_LeftArrow;
    LCD_setText(lcd, 2, opts, (value->valueString)? value->valueString: "(null)");
}

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
    value->accessType  = ValueAccessType_Manual;
    value->valuePtr    = NULL;
    value->callback    = NULL;
    List_insertLast(&debugValues, List_newNode(value));

    // add the event handlers //
    if(debugValues.nodeCount == 1) {
        VexOS_addEventHandler(EventType_DisabledPeriodic,   &pullValues, NULL);
        VexOS_addEventHandler(EventType_AutonomousPeriodic, &pullValues, NULL);
        VexOS_addEventHandler(EventType_OperatorPeriodic,   &pullValues, NULL);
    }
    return value;
}

DebugValue* DebugValue_delete(DebugValue* value) {
    ErrorIf(value == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&debugValues, value);
    if(node == NULL) return value;
    if(currentValue == node) setCurrentValue(NULL);
    List_remove(node);
    // check for last value and remove handlers //
    if(debugValues.nodeCount == 0) {
        VexOS_removeEventHandler(EventType_DisabledPeriodic,   &pullValues);
        VexOS_removeEventHandler(EventType_AutonomousPeriodic, &pullValues);
        VexOS_removeEventHandler(EventType_OperatorPeriodic,   &pullValues);
    }
    free((void*) value->valueString);
    free(value);
    return NULL;
}

void DebugValue_setPointer(DebugValue* value, void* valuePtr) {
    value->accessType = (valuePtr)? ValueAccessType_Pointer: ValueAccessType_Manual;
    value->valuePtr   = valuePtr;
    value->callback   = NULL;
}

void DebugValue_setCallback(DebugValue* value, DebugValueCallback* callback) {
    value->accessType = (callback)? ValueAccessType_Callback: ValueAccessType_Manual;
    value->valuePtr   = NULL;
    value->callback   = callback;
}

void DebugValue_set(DebugValue* value, ...) {
    ErrorIf(value == NULL, VEXOS_ARGNULL);
    
    // make sure we haven't changed within sample time //
    unsigned long time = GetMsClock();
    if(value->changeTime >= (time - SAMPLE_TIME)) return;

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
        value->changeTime  = time;
    } else if(value->valueType != DebugValueType_Bool) {
        // not different, free new string if we allocated it //
        free(vstring);
    }
}

/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

Window* DebugValue_getWindow() {
    static Window* window = NULL;
    if(window) return window;
    window = Window_new("Debug Values", &updateWindow);
    Window_setSize(window, 27, 15);
    return window;
}

LCDScreen* DebugValue_getLCDScreen() {
    static LCDScreen* screen = NULL;
    if(screen) return screen;
    screen = LCDScreen_new("Debug Values", &drawLCDScreen, &getLCDStatus);
    return screen;
}
