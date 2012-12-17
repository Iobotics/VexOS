//
//  VexOS.c
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

#include "UserInterface.h"
#include "Scheduler.h"
#include "Error.h"

/********************************************************************
 * Private API: General                                             *
 ********************************************************************/

// internal headers //
extern void Subsystem_initialize();

// event handling //
static List* events[EVENT_COUNT];

typedef struct {
    EventHandler* handler;
    void*         state;
} EventData;

// run loop processing //
#define LOOP_SAMPLE     100
static RunMode       runMode;
static unsigned long lastTime;
static unsigned long loopTime;
static unsigned int  loopCount;

// program name //
static String programName;

/********************************************************************
 * Private API: Run Loops                                           *
 ********************************************************************/

static void loopStart() {
    loopCount = 0;
    Dashboard_refresh();
}

static void loopPeriodic() {
    // handle global timing //
    unsigned long time = GetMsClock();
    if(loopCount++ == LOOP_SAMPLE) {
        loopTime = (time - lastTime);
        lastTime = time;
        loopCount = 0;
    }
    // run command scheduler //
    Scheduler_run();
}

static void fireEvent(EventType type) {
    if(events[type] == NULL) return;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        EventData* data = node->data;
        data->handler(type, data->state);
        node = node->next;
    }
}

static void executeLoop(RunMode mode) {
    ErrorCode err;
    if(!(err = setjmp(ErrorEvn))) {
        bool enabled = true;
        EventType start, periodic;
        switch(mode) {
            case RunMode_Initialize:
                // setup Subsystems //
                runMode = RunMode_VexOS_Setup;
                Subsystem_initialize();
                runMode = RunMode_Initialize;
                InitializeRobot();
                // set loop parameters //
                start    = EventType_DisabledStart;
                periodic = EventType_DisabledPeriodic;
                enabled  = false;
                break;
            case RunMode_Autonomous:
                runMode  = RunMode_Autonomous;
                start    = EventType_AutonomousStart;
                periodic = EventType_AutonomousPeriodic;
                break;
            case RunMode_Operator:
                runMode  = RunMode_Operator;
                start    = EventType_OperatorStart;
                periodic = EventType_OperatorPeriodic;
                break;
            default:
                ErrorMsgIf(true, VEXOS_OPINVALID, "Invalid RunMode: %d", mode);
                return;
        }
        // start and execute the loop //
        loopStart();
        fireEvent(start);
        while(IsEnabled() == enabled) {
            loopPeriodic();
            fireEvent(periodic);
        }
    } else {
        // error handling trap //
        Error_setCode(err);
        Info(Error_getMessage());
        fireEvent(EventType_SystemError);
        while(true) Wait(1000);
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

RunMode VexOS_getRunMode() {
    return runMode;
}

unsigned long VexOS_getRunTime() {
    return (unsigned long) (lastTime / 1000);
}

double VexOS_getLoopFrequency() {
    return (1000.0 * LOOP_SAMPLE / loopTime);
}

String VexOS_getProgramName() {
    return programName;
}

void VexOS_setProgramName(String name) {
    programName = name;
}

bool VexOS_addEventHandler(EventType type, EventHandler* handler, void* state) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_SystemError,
               VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);

    // check if already there //
    if(VexOS_hasEventHandler(type, handler)) return false;
    // add list if not there //
    if(events[type] == NULL) {
        events[type] = List_new();
    }
    // add handler to list //
    EventData* data = malloc(sizeof(EventData));
    data->handler = handler;
    data->state   = state;
    List_insertLast(events[type], List_newNode(data));
    return true;
}

bool VexOS_removeEventHandler(EventType type, EventHandler* handler) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_SystemError,
               VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);
    
    if(events[type] == NULL) return false;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        EventData* data = node->data;
        if(data->handler == handler) {
            List_remove(node);
            free(node);
            return true;
        }
        node = node->next;
    }
    return false;
}

bool VexOS_hasEventHandler(EventType type, EventHandler* handler) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_SystemError,
               VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);
    
    if(events[type] == NULL) return false;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        EventData* data = (EventData*) node->data;
        if(data->handler == handler) return true;
        node = node->next;
    }
    return false;
}

void VexOS_setupStandardUI() {
    Dashboard* mainDash = Dashboard_new("Main");
    Dashboard_addWindowWithPosition(mainDash, Status_getWindow(),      0, 0);
    Dashboard_addWindowWithPosition(mainDash, Battery_getWindow(),    24, 0);
    Dashboard_addWindowWithPosition(mainDash, Autonomous_getWindow(), 48, 0);
    Dashboard_addWindowWithPosition(mainDash, Scheduler_getWindow(),   0, 7);
    Dashboard_addWindowWithPosition(mainDash, DebugValue_getWindow(), 48, 9);
    Dashboard* hdwDash = Dashboard_new("Hardware");
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_Digital),  0,  0);
    /*Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_Analog),   0, 14);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_PWM),     24,  0);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_UART),    24, 14);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_I2C),     48,  0);*/
    Dashboard_restoreLast();
    
    LCD* lcd = (LCD*) Device_getByType(DeviceType_LCD);
    if(lcd) {
        LCD_addScreen(lcd, Status_getLCDScreen());
        LCD_addScreen(lcd, Battery_getLCDScreen());
        LCD_addScreen(lcd, Autonomous_getLCDScreen());
        LCD_addScreen(lcd, Dashboard_getLCDScreen());
        LCD_addScreen(lcd, DebugValue_getLCDScreen());
        LCD_restoreLastScreen(lcd);
    }
}

/********************************************************************
 * easyC Entry Points                                               *
 ********************************************************************/

// main entry point from easyC, always called //
void VexOS_Initialize() {
    Info("VexOS Initialize\n\n");
    executeLoop(RunMode_Initialize);
}

void VexOS_OperatorControl(unsigned long ulTime) {
    Info("OperatorControl\n");
    executeLoop(RunMode_Operator);
}

void VexOS_Autonomous(unsigned long ulTime) {
    Info("Autonomous\n");
    executeLoop(RunMode_Autonomous);    
}
