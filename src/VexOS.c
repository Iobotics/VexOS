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

#include "API.h"

#include "Interrupt.h"
#include "UserInterface.h"
#include "Scheduler.h"
#include "Error.h"

/********************************************************************
 * Private API: General                                             *
 ********************************************************************/

// internal headers //
extern const struct Robot Robot;
extern void Subsystem_construct();
extern void Subsystem_initialize();
extern void Device_configureCortex();

// event handling //
static List* events[EVENT_COUNT];

typedef struct {
    EventHandler* handler;
    void*         state;
} EventData;

// run loop processing //
#define LOOP_SAMPLE     100
static RunMode       runMode;
static RunMode       lastRunMode;
static unsigned long lastTime;
static unsigned long loopTime;
static unsigned int  loopCount;

// program name //
static String programName;

/********************************************************************
 * Private API: Run Loops                                           *
 ********************************************************************/

static void setRunMode(RunMode mode) {
    runMode = mode;
    GlobalData(GLOBALDATA_LAST_RUN_MODE) = mode;
}

static void loopStart() {
    loopCount = 0;
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
        EventType start, periodic, end;
        switch(mode) {
            case RunMode_Initialize:
                // setup Subsystems and hardware //
                lastRunMode = GlobalData(GLOBALDATA_LAST_RUN_MODE);
                setRunMode(RunMode_Setup);
                Subsystem_construct();
                Robot.constructor();
                Device_configureCortex();
                Interrupt_enable();
                // run robot initializer //
                setRunMode(RunMode_Initialize);
                Subsystem_initialize();
                Robot.initialize();
                // set loop parameters //
                start    = EventType_Initialize;
                periodic = EventType_DisabledPeriodic;
                end      = EventType_DisabledEnd;
                enabled  = false;
                break;
            case RunMode_Autonomous:
                setRunMode(RunMode_Autonomous);
                start    = EventType_AutonomousStart;
                periodic = EventType_AutonomousPeriodic;
                end      = -1;
                break;
            case RunMode_Operator:
                setRunMode(RunMode_Operator);
                start    = EventType_OperatorStart;
                periodic = EventType_OperatorPeriodic;
                end      = -1;
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
        if(end > -1) fireEvent(end);
    } else {
        // error handling trap //
        Interrupt_disable();
        Wait(500);
        Error_setCode(err);
        Info("****\n");
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

RunMode VexOS_getLastRunMode() {
    return lastRunMode;
}

unsigned long VexOS_getRunTime() {
    return (unsigned long) (lastTime / 1000);
}

float VexOS_getLoopFrequency() {
    return (1000.0 * LOOP_SAMPLE / loopTime);
}

String VexOS_getProgramName() {
    return programName;
}

void VexOS_setProgramName(String name) {
    programName = name;
}

bool VexOS_addEventHandler(EventType type, EventHandler* handler, void* state) {
    ErrorMsgIf(type < EventType_Initialize || type > EventType_SystemError,
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
    ErrorMsgIf(type < EventType_Initialize || type > EventType_SystemError,
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
    ErrorMsgIf(type < EventType_Initialize || type > EventType_SystemError,
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
    ErrorIf(VexOS_getRunMode() == RunMode_Setup, VEXOS_NOTINITIALIZED);

    Dashboard* mainDash = Dashboard_new("Main");
    Dashboard_addWindowWithPosition(mainDash, Status_getWindow(),      0, 0);
    Dashboard_addWindowWithPosition(mainDash, Battery_getWindow(),    24, 0);
    Dashboard_addWindowWithPosition(mainDash, Autonomous_getWindow(), 48, 0);
    Dashboard_addWindowWithPosition(mainDash, Scheduler_getWindow(),   0, 7);
    Dashboard_addWindowWithPosition(mainDash, DebugValue_getWindow(), 48, 9);
    Dashboard* hdwDash = Dashboard_new("Hardware");
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_Digital),  0,  0);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_Analog),   0, 16);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_PWM),     34,  0);
    Dashboard_addWindowWithPosition(hdwDash, Device_getWindow(DeviceWindowType_UART),    34, 14);
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
