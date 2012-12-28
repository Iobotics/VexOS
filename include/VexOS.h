//
//  VexOS.h
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

#ifndef _VexOS_h
#define _VexOS_h

#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/********************************************************************
 * Operating System Constants                                       *
 ********************************************************************/

#define VEXOS_MAJOR_VERSION     0
#define VEXOS_MINOR_VERSION     9
#define VEXOS_BUILD_VERSION     6

/********************************************************************
 * Reserved GlobalData Slots                                        *
 ********************************************************************/

// global data slots //
#define GLOBALDATA_AUTO_PROGRAM     20
#define GLOBALDATA_DASH_NUMBER      19
#define GLOBALDATA_LCD_SCREEN       18
#define GLOBALDATA_DEBUG_VALUE      17
// saved non-IME MotorGroup PID positions grow upwards from this //
// value. this means there are effectively 5 slots available if  //
// the other VexOS state values are used.                        //
#define GLOBALDATA_MOTORGROUP_STATE 12

/********************************************************************
 * Public Object Definitions                                        *
 ********************************************************************/

// main objects //
typedef struct Subsystem     Subsystem;
typedef struct CommandClass  CommandClass;
typedef struct Command       Command;
typedef struct ButtonClass   ButtonClass;
typedef struct Button        Button;

// utility objects //
typedef struct Joystick      Joystick;
typedef struct PowerScaler   PowerScaler;
typedef struct DebugValue    DebugValue;
typedef struct PIDController PIDController;
typedef struct Timer         Timer;

// scalar types //
typedef const char* String;
typedef float       Power;

/********************************************************************
 * Public API: Operating System Functions and Events                *
 ********************************************************************/

typedef enum {
    RunMode_Setup,
    RunMode_Initialize,
    RunMode_Autonomous,
    RunMode_Operator
} RunMode;

#define EVENT_COUNT     8

typedef enum {
    EventType_Initialize,
    EventType_DisabledPeriodic,
    EventType_DisabledEnd,
    EventType_AutonomousStart,
    EventType_AutonomousPeriodic,
    EventType_OperatorStart,
    EventType_OperatorPeriodic,
    EventType_SystemError
} EventType;

typedef void (EventHandler)(EventType, void*);

// main system functions //
RunMode       VexOS_getRunMode();
unsigned long VexOS_getRunTime();
float         VexOS_getLoopFrequency();
String        VexOS_getProgramName();
void          VexOS_setProgramName(String);

// event handling //
bool VexOS_addEventHandler(EventType, EventHandler*, void*);
bool VexOS_removeEventHandler(EventType, EventHandler*);
bool VexOS_hasEventHandler(EventType, EventHandler*);

// standard user interface setup //
void VexOS_setupStandardUI();

/********************************************************************
 * Public API: Autonomous                                           *
 ********************************************************************/

unsigned int Autonomous_addProgram(Command*);
bool         Autonomous_removeProgram(Command*);
bool         Autonomous_hasProgram(Command*);
unsigned int Autononous_getProgramCount();
Command*     Autonomous_getProgramByNumber(unsigned int n);
Command*     Autonomous_getSelectedProgram();
void         Autonomous_setSelectedProgram(Command* cmd);
void         Autonomous_setSelectedProgramByNumber(unsigned int n);
bool         Autonomous_restoreLastProgram();

/********************************************************************
 * Public API: Battery                                              *
 ********************************************************************/

#define BatteryThreshold_MAIN_G     7.5
#define BatteryThreshold_MAIN_Y     7.0
#define BatteryThreshold_BACKUP_G   7.0
#define BatteryThreshold_BACKUP_Y   6.5

void Battery_setMainThresholds(float  threshG, float  threshY);
void Battery_getMainThresholds(float* threshG, float* threshY);
void Battery_setBackupThresholds(float  threshG, float  threshY);
void Battery_getBackupThresholds(float* threshG, float* threshY);

/********************************************************************
 * Public API: CommandClass                                         *
 ********************************************************************/

String CommandClass_getName(CommandClass*);
bool   CommandClass_isInitialized(CommandClass*);

/********************************************************************
 * Public API: Command                                              *
 ********************************************************************/

typedef enum {
    CommandStatus_Running     = 0x01,
    CommandStatus_Cancelled   = 0x02,
    CommandStatus_Locked      = 0x04,
    CommandStatus_Initialized = 0x08
} CommandStatus;

Command*      Command_new(CommandClass*, ...);
Command*      Command_delete(Command*);
void          Command_start(Command*);
void          Command_cancel(Command*);
String        Command_getName(Command*);
CommandClass* Command_getClass(Command*);
Command*      Command_getParent(Command*);
CommandStatus Command_getStatus(Command*);
bool          Command_isInterruptible(Command*);
bool          Command_runWhenDisabled(Command*);
void          Command_setRunWhenDisabled(Command*, bool);
bool          Command_doesRequireSubsystem(Command*, Subsystem*);
unsigned long Command_timeSinceInitialized(Command*);

/********************************************************************
 * Public API: CommandGroup                                         *
 ********************************************************************/

// shares all methods with Command //
void CommandGroup_addSequential(Command*, Command*);
void CommandGroup_addSequentialWithTimeout(Command*, Command*, unsigned long);
void CommandGroup_addParallel(Command*, Command*);
void CommandGroup_addParallelWithTimeout(Command*, Command*, unsigned long);

/********************************************************************
 * Public API: ButtonClass                                          *
 ********************************************************************/

String ButtonClass_getName(ButtonClass*);
bool   ButtonClass_isInitialized(ButtonClass*);

/********************************************************************
 * Public API: Button                                               *
 ********************************************************************/

Button*      Button_new(ButtonClass*, ...);
Button*      Button_delete(Button*);
String       Button_getName(Button*);
ButtonClass* Button_getClass(Button*);
bool         Button_get(Button*);
void         Button_whenPressed(Button*, Command*);
void         Button_whileHeld(Button*, Command*);
void         Button_whenReleased(Button*, Command*);
void         Button_whileToggled(Button*, Command*);
int          Button_getToggleGroup(Button*);
void         Button_setToggleGroup(Button*, int);

/********************************************************************
 * Public API: InternalButton                                       *
 ********************************************************************/

void InternalButton_set(Button*, bool);

/********************************************************************
 * Public API: Subsystem                                            *
 ********************************************************************/

String   Subsystem_getName(Subsystem*);
bool     Subsystem_isInitialized(Subsystem*);
Command* Subsystem_getCurrentCommand(Subsystem*);
Command* Subsystem_getDefaultCommand(Subsystem*);

/********************************************************************
 * Public API: Joystick                                             *
 ********************************************************************/

typedef enum {
    JoystickHand_Left,
    JoystickHand_Right
} JoystickHand;

typedef enum {
    JoystickButtonType_Down = 1,
    JoystickButtonType_Up,
    JoystickButtonType_Left,
    JoystickButtonType_Right,
    JoystickButtonType_Bottom,
    JoystickButtonType_Top
} JoystickButtonType;

Joystick*     Joystick_new(unsigned char);
Joystick*     Joystick_delete(Joystick*);
unsigned char Joystick_getId(Joystick*);
float         Joystick_getX(Joystick*, JoystickHand);
float         Joystick_getY(Joystick*, JoystickHand);
void          Joystick_setXDeadband(Joystick*, JoystickHand, float, float); 
void          Joystick_setYDeadband(Joystick*, JoystickHand, float, float);
Button*       Joystick_getButton(Joystick*, JoystickHand, JoystickButtonType);
bool          Joystick_getRawButton(Joystick*, JoystickHand, JoystickButtonType);
float         Joystick_getAccelerometerX(Joystick*);
float         Joystick_getAccelerometerY(Joystick*);
void          Joystick_setAccelerometerXDeadband(Joystick*, float, float); 
void          Joystick_setAccelerometerYDeadband(Joystick*, float, float);
    
/********************************************************************
 * Public API: PowerScaler                                          *
 ********************************************************************/

PowerScaler* PowerScaler_new(String);
PowerScaler* PowerScaler_delete(PowerScaler*);
String       PowerScaler_getName(PowerScaler*);
void         PowerScaler_addPoint(PowerScaler*, Power, Power);
Power        PowerScaler_get(PowerScaler*, Power);
String       PowerScaler_toString(PowerScaler*);

/********************************************************************
 * Public API: DebugValue                                           *
 ********************************************************************/

typedef enum {
    DebugValueType_Int = 1,
    DebugValueType_String,
    DebugValueType_Float,
    DebugValueType_Bool,
    DebugValueType_Format
} DebugValueType;

DebugValue* DebugValue_new(String, DebugValueType);
DebugValue* DebugValue_newWithFormat(String, DebugValueType, String);
DebugValue* DebugValue_delete(DebugValue*);
void        DebugValue_set(DebugValue*, ...);

/********************************************************************
 * Public API: PIDController                                        *
 ********************************************************************/

#define PID_PERIOD  0.05

typedef float (PIDInput)(void*);
typedef void (PIDOutput)(void*, float);

PIDController* PIDController_new(float, float, float, PIDInput, PIDOutput, void*);
PIDController* PIDController_delete(PIDController*);
void*   PIDController_getState(PIDController*);
float   PIDController_get(PIDController*);
float   PIDController_getError(PIDController*);
float   PIDController_getP(PIDController*);
float   PIDController_getI(PIDController*);
float   PIDController_getD(PIDController*);
float   PIDController_getSetpoint(PIDController*);
bool    PIDController_onTarget(PIDController*);
void    PIDController_reset(PIDController*);
bool    PIDController_isContinuous(PIDController*);
void    PIDController_setContinuous(PIDController*, bool);
bool    PIDController_isEnabled(PIDController*);
void    PIDController_setEnabled(PIDController*, bool);
void    PIDController_setInputRange(PIDController*, float, float);
void    PIDController_setOutputRange(PIDController*, float, float);
void    PIDController_setPID(PIDController*, float, float, float);
void    PIDController_setSetpoint(PIDController*, float);
void    PIDController_setTolerance(PIDController*, float);

/********************************************************************
 * Public API: Timer                                                  *
 ********************************************************************/

#define TIMER_COUNT 6
typedef enum {
    Timer_1 = 1, Timer_2, Timer_3, Timer_4, Timer_5, Timer_6
} TimerId;

Timer*        Timer_new(String, TimerId);
Timer*        Timer_delete(Timer*);
String        Timer_getName(Timer*);
bool          Timer_isEnabled(Timer*);
void          Timer_setEnabled(Timer*, bool);
void          Timer_preset(Timer*, unsigned long);
unsigned long Timer_get(Timer*);

/********************************************************************
 * Public API: List                                                 *
 ********************************************************************/

struct ListNode_T;
typedef struct List_T {
    unsigned int        nodeCount;
    struct ListNode_T*  firstNode;
    struct ListNode_T*  lastNode;
} List;

typedef struct ListNode_T {
    void*               data;
    struct ListNode_T*  prev;
    struct ListNode_T*  next;
    struct List_T*      list;
} ListNode;

List*        List_new();
List*        List_delete(List*);
ListNode*    List_newNode(void* data);
void         List_insertAfter(ListNode* node, ListNode* newNode);
void         List_insertBefore(ListNode* node, ListNode* newNode);
void         List_insertFirst(List* list, ListNode* newNode);
void         List_insertLast(List* list, ListNode* newNode);
ListNode*    List_remove(ListNode* node);
ListNode*    List_findNode(List*, void*);
unsigned int List_indexOfNode(ListNode*);
unsigned int List_indexOfData(List*, void*);
ListNode*    List_getNodeByIndex(List*, unsigned int n);
void*        List_getDataByIndex(List*, unsigned int n);

/********************************************************************
 * Robot Configuration Macros                                       *
 ********************************************************************/

struct Robot {
    Subsystem** subsystems;
    void (*constructor)();
    void (*initialize)();
};

#define DefineRobot(...) \
    static Subsystem* subsystems[] = { __VA_ARGS__, NULL }; \
    static void constructor(); \
    static void initialize(); \
    const struct Robot Robot = { \
        .subsystems  = subsystems, \
        .constructor = &constructor, \
        .initialize  = &initialize \
    };

#define DeclareSubsystem(class) \
    extern Subsystem class;

#define DeclareCommandClass(class) \
    extern CommandClass class;

#define DeclareButtonClass(class) \
    extern ButtonClass class;


/********************************************************************
 * Built-in Generic Command & Button Classes                        *
 ********************************************************************/

// commands //
DeclareCommandClass(CommandGroup);
DeclareCommandClass(PrintCommand);
DeclareCommandClass(StartCommand);
DeclareCommandClass(WaitCommand);
DeclareCommandClass(WaitForChildren);
DeclareCommandClass(WaitUntilCommand);

// buttons //
DeclareButtonClass(DigitalIOButton);
DeclareButtonClass(InternalButton);
DeclareButtonClass(JoystickButton);

#endif // _VexOS_H
