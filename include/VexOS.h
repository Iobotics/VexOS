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
#include <math.h>

/********************************************************************
 * Operating System Constants                                       *
 ********************************************************************/

#define VEXOS_MAJOR_VERSION     0
#define VEXOS_MINOR_VERSION     9
#define VEXOS_BUILD_VERSION     7

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
void          VexOS_setProgramName(String name);

// event handling //
bool VexOS_addEventHandler(EventType type, EventHandler* handler, void* state);
bool VexOS_removeEventHandler(EventType type, EventHandler* handler);
bool VexOS_hasEventHandler(EventType type, EventHandler* handler);

// standard user interface setup //
void VexOS_setupStandardUI();

/********************************************************************
 * Public API: Autonomous                                           *
 ********************************************************************/

unsigned int Autonomous_addProgram(Command* programCmd);
bool         Autonomous_removeProgram(Command* programCmd);
bool         Autonomous_hasProgram(Command* programCmd);
unsigned int Autononous_getProgramCount();
Command*     Autonomous_getProgramByNumber(unsigned int n);
Command*     Autonomous_getSelectedProgram();
void         Autonomous_setSelectedProgram(Command* programCmd);
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

String CommandClass_getName(CommandClass* class);
bool   CommandClass_isInitialized(CommandClass* class);

/********************************************************************
 * Public API: Command                                              *
 ********************************************************************/

typedef enum {
    CommandStatus_Running     = 0x01,
    CommandStatus_Cancelled   = 0x02,
    CommandStatus_Locked      = 0x04,
    CommandStatus_Initialized = 0x08
} CommandStatus;

Command*      Command_new(CommandClass* class, ...);
Command*      Command_delete(Command* cmd);
void          Command_start(Command* cmd);
void          Command_cancel(Command* cmd);
String        Command_getName(Command* cmd);
CommandClass* Command_getClass(Command* cmd);
Command*      Command_getParent(Command* cmd);
CommandStatus Command_getStatus(Command* cmd);
bool          Command_isInterruptible(Command* cmd);
bool          Command_runWhenDisabled(Command* cmd);
void          Command_setRunWhenDisabled(Command* cmd, bool value);
bool          Command_doesRequireSubsystem(Command* cmd, Subsystem* sys);
float         Command_timeSinceInitialized(Command* cmd);

/********************************************************************
 * Public API: CommandGroup                                         *
 ********************************************************************/

// shares all methods with Command //
void CommandGroup_addSequential(Command* group, Command* cmd);
void CommandGroup_addSequentialWithTimeout(Command* group, Command* cmd, float timeout);
void CommandGroup_addParallel(Command* group, Command* cmd);
void CommandGroup_addParallelWithTimeout(Command* group, Command* cmd, float timeout);

/********************************************************************
 * Public API: ButtonClass                                          *
 ********************************************************************/

String ButtonClass_getName(ButtonClass* class);
bool   ButtonClass_isInitialized(ButtonClass* class);

/********************************************************************
 * Public API: Button                                               *
 ********************************************************************/

Button*      Button_new(ButtonClass* class, ...);
Button*      Button_delete(Button* button);
String       Button_getName(Button* button);
ButtonClass* Button_getClass(Button* button);
bool         Button_get(Button* button);
void         Button_whenPressed(Button* button, Command* cmd);
void         Button_whileHeld(Button* button, Command* cmd);
void         Button_whenReleased(Button* button, Command* cmd);
void         Button_whileToggled(Button* button, Command* cmd);
int          Button_getToggleGroup(Button* button);
void         Button_setToggleGroup(Button* button, int groupId);

/********************************************************************
 * Public API: InternalButton                                       *
 ********************************************************************/

void InternalButton_set(Button* button, bool value);

/********************************************************************
 * Public API: Subsystem                                            *
 ********************************************************************/

String   Subsystem_getName(Subsystem* sys);
bool     Subsystem_isInitialized(Subsystem* sys);
Command* Subsystem_getCurrentCommand(Subsystem* sys);
Command* Subsystem_getDefaultCommand(Subsystem* sys);

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

Joystick*     Joystick_new(unsigned char joystickId);
Joystick*     Joystick_delete(Joystick* joystick);
unsigned char Joystick_getId(Joystick* joystick);
float         Joystick_getX(Joystick* joystick, JoystickHand hand);
float         Joystick_getY(Joystick* joystick, JoystickHand hand);
void          Joystick_setXDeadband(Joystick* joystick, JoystickHand hand, float min, float max); 
void          Joystick_setYDeadband(Joystick* joystick, JoystickHand hand, float min, float max);
Button*       Joystick_getButton(Joystick* joystick, JoystickHand hand, JoystickButtonType type);
bool          Joystick_getRawButton(Joystick* joystick, JoystickHand hand, JoystickButtonType type);
float         Joystick_getAccelerometerX(Joystick* joystick);
float         Joystick_getAccelerometerY(Joystick* joystick);
void          Joystick_setAccelerometerXDeadband(Joystick* joystick, float min, float max); 
void          Joystick_setAccelerometerYDeadband(Joystick* joystick, float min, float max);
    
/********************************************************************
 * Public API: PowerScaler                                          *
 ********************************************************************/

PowerScaler* PowerScaler_new(String name);
PowerScaler* PowerScaler_delete(PowerScaler* scaler);
String       PowerScaler_getName(PowerScaler* scaler);
void         PowerScaler_addPoint(PowerScaler* scaler, Power xIn, Power yOut);
Power        PowerScaler_get(PowerScaler* scaler, Power in);
String       PowerScaler_toString(PowerScaler* scaler);

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

DebugValue* DebugValue_new(String name, DebugValueType type);
DebugValue* DebugValue_newWithFormat(String name, DebugValueType type, String fmtString);
DebugValue* DebugValue_delete(DebugValue* val);
void        DebugValue_set(DebugValue* val, ...);

/********************************************************************
 * Public API: PIDController                                        *
 ********************************************************************/

#define PID_PERIOD  0.05

typedef float (PIDInput)(void* state);
typedef void (PIDOutput)(void* state, float result);

PIDController* PIDController_new(PIDInput inHandler, PIDOutput outHandler, void* state);
PIDController* PIDController_delete(PIDController* pid);
void*   PIDController_getState(PIDController* pid);
void    PIDController_setPID(PIDController* pid, float kP, float kI, float kD);
float   PIDController_getP(PIDController* pid);
float   PIDController_getI(PIDController* pid);
float   PIDController_getD(PIDController* pid);
void    PIDController_setInputRange(PIDController* pid, float min, float max);
void    PIDController_setOutputRange(PIDController* pid, float min, float max);
bool    PIDController_isContinuous(PIDController* pid);
void    PIDController_setContinuous(PIDController* pid, bool value);
bool    PIDController_isEnabled(PIDController* pid);
void    PIDController_setEnabled(PIDController* pid, bool value);
void    PIDController_setTolerance(PIDController* pid, float tolerance);
bool    PIDController_onTarget(PIDController* pid);
float   PIDController_getError(PIDController* pid);
float   PIDController_getOutput(PIDController* pid);
float   PIDController_getSetpoint(PIDController* pid);
void    PIDController_setSetpoint(PIDController* pid, float setpoint);

/********************************************************************
 * Public API: Timer                                                  *
 ********************************************************************/

#define TIMER_COUNT 6
typedef enum {
    Timer_1 = 1, Timer_2, Timer_3, Timer_4, Timer_5, Timer_6
} TimerId;

Timer* Timer_new(String name, TimerId timeId);
Timer* Timer_delete(Timer* timer);
String Timer_getName(Timer* timer);
bool   Timer_isEnabled(Timer* timer);
void   Timer_setEnabled(Timer* timer, bool value);
void   Timer_preset(Timer* timer, float);
float  Timer_get(Timer* timer);

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
List*        List_delete(List* list);
ListNode*    List_newNode(void* data);
void         List_insertAfter(ListNode* node, ListNode* newNode);
void         List_insertBefore(ListNode* node, ListNode* newNode);
void         List_insertFirst(List* list, ListNode* newNode);
void         List_insertLast(List* list, ListNode* newNode);
ListNode*    List_remove(ListNode* node);
ListNode*    List_findNode(List* list, void* data);
unsigned int List_indexOfNode(ListNode* node);
unsigned int List_indexOfData(List* list, void* data);
ListNode*    List_getNodeByIndex(List* list, unsigned int n);
void*        List_getDataByIndex(List* list, unsigned int n);

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
 * Utility Macros                                                   *
 ********************************************************************/

#define ABS(x) ({ typeof(x) _x = (x); (_x < 0)? -_x: _x; })
#define MAX(x, y) ({ typeof(x) _x = (x); typeof(y) _y = (y); (_x > _y)? _x: _y)
#define MIN(x, y) ({ typeof(x) _x = (x); typeof(y) _y = (y); (_x < _y)? _x: _y)

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
