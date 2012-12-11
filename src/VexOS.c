//
//  VexOS.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Dashboard.h"
#include "Error.h"
#include "LCD.h"
#include "Scheduler.h"
#include "Window.h"

/********************************************************************
 * Private API: General                                             *
 ********************************************************************/

// event handling //
#define EVENT_COUNT     7
static List* events[EVENT_COUNT];

// run loop processing //
#define LOOP_SAMPLE     100
static RunMode runMode;
static unsigned long lastTime;
static unsigned long loopTime;
static unsigned int  loopCount;

// program name //
static const char* programName;

// UI sequencing //
#define DASH_PERIOD     50
#define LCD_PERIOD      100
static unsigned long nextDashTime;
static unsigned long nextLCDTime;

// power expander handling //
static AnalogPort        expanderPort;
//static PowerExpanderType expanderType;

// autonomous programs //
static List autonomousPrograms;
static ListNode* activeAutonomous;
static bool autoChanged;

void setActiveAutonomous(ListNode* node) {
    activeAutonomous = node;
    autoChanged      = true;
    // store in global data //
    GlobalData(GLOBALDATA_AUTO_PROGRAM) = (node)? (List_indexOfNode(node) + 1): 0;
}

bool restoreActiveAutonomous() {
    unsigned int value = (unsigned int) GlobalData(GLOBALDATA_AUTO_PROGRAM);
    if(value > 0) {
        VexOS_setSelectedAutonomousByNumber(value);
        return true;
    }
    return false;
}

/********************************************************************
 * Private API: Dashboard User Interface                            *
 ********************************************************************/

static bool dashboardInitialized;

// update the status window //
static void statusWinUpdate(Window* win, bool full) {
    unsigned char left = win->innerRect.left;
    unsigned char top  = win->innerRect.top;
    // loop frequency //
    PrintTextToGD(top, left, Color_Black, "Loop: %4.0f Hz \n", VexOS_getLoopFrequency());
    // run mode //
    if(full) {
        switch(runMode) {
            case RunMode_VexOS_Setup:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Setup\n");
                break;
            case RunMode_Initialize:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Disabled\n");
                break;
            case RunMode_Autonomous:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Auto\n");
                break;
            case RunMode_Operator:
                PrintTextToGD(top + 1, left, Color_Black, "Mode: Operator\n");
                break;
        }
    }
    // run time //
    PrintTextToGD(top + 2, left, Color_Black, "Run:  %d s\n", VexOS_getRunTime());
}

// get the battery color based on thresholds //
static Color getBatteyColor(float volts, bool backup) {
    /*if(backup) {
        if(volts >= BatteryThreshold_BACKUP_G) return Color_DarkGreen;
        if(volts >= BatteryThreshold_BACKUP_Y) return Color_DarkYellow;
        return Color_Red;
    } else {
        if(volts >= BatteryThreshold_MAIN_G) return Color_DarkGreen;
        if(volts >= BatteryThreshold_MAIN_Y) return Color_DarkYellow;
        return Color_Red;
    }*/
    return Color_Red;
}

// update the battery window //
static void batteryWinUpdate(Window* win, bool full) {
    unsigned char left = win->innerRect.left;
    unsigned char top  = win->innerRect.top;
    float volts;
    // main battery //
    volts = GetMainBattery();
    PrintTextToGD(top, left, Color_Black, "Main:\n");
    PrintTextToGD(top++, left + 10, getBatteyColor(volts, false), "%1.2f V\n", volts);
    // expander battery //
    if(expanderPort > 0) {
        //volts = GetExpanderBattery();
        PrintTextToGD(top,   left, Color_Black, "Expander:\n");
        PrintTextToGD(top++, left + 10, getBatteyColor(volts, false), "%1.2f V\n", volts);
    }
    // backup battery //
    volts = GetBackupBattery();
    PrintTextToGD(top,   left, Color_Black, "Backup:\n");
    PrintTextToGD(top++, left + 10, getBatteyColor(volts, true), "%1.2f V\n", volts);
}

// update the autonomous program window //
static void autoWinUpdate(Window* win, bool full) {
    unsigned char left = win->innerRect.left;
    unsigned char top  = win->innerRect.top;
    
    if(!full && !autoChanged) return;
    ClearGD(top, left, win->innerRect.bottom, win->innerRect.right, false);
    
    Command* selected = VexOS_getSelectedAutonomous();
    int selLine = 0;
    
    // print the (none) choice //
    PrintTextToGD(top, left + 1, Color_Black, "(none)\n");
    if(selected == NULL) selLine = top;
    top++;
    
    // print the stored programs //
    ListNode* node = autonomousPrograms.firstNode;
    while(node != NULL) {
        Command* cmd = node->data;
        PrintTextToGD(top, left + 1, Color_Black, "%.*s\n", win->width, Command_getName(cmd));
        if(selected == cmd) selLine = top;
        top++;
        node = node->next;
    }
    
    // print the selection mark //
    PrintTextToGD(selLine, left, Color_Black, ">\n");
    autoChanged = false;
}

// create the windows and add to the dashboard //
static void initializeDashboard() {
    Dashboard_addWindow(Window_new(16, 3, "Robot Status", &statusWinUpdate), 0, 0);
    Dashboard_addWindow(Window_new(16, 3, "Battery", &batteryWinUpdate), 24, 0);
    Dashboard_addWindow(Window_new(27, 5, "Autonomous Program", &autoWinUpdate), 48, 0);
    Dashboard_addWindow(Scheduler_getWindow(), 0, 7);
    Dashboard_addWindow(DebugValue_getWindow(), 48, 9);
    dashboardInitialized = true;
    Dashboard_restoreEnabled();
}

/********************************************************************
 * Private API: LCD User Interface                                  *
 ********************************************************************/

static bool lcdInitialized;

// handle the title (main) LCD menu //
static void drawLCDTitleScreen(LCDScreen* screen, LCDButtonType pressed) {
    const char* modeStr;
    switch(VexOS_getRunMode()) {
        case RunMode_VexOS_Setup:   modeStr = "VexOS";    break;
        case RunMode_Initialize:    modeStr = "Disabled"; break;
        case RunMode_Autonomous:    modeStr = "Auto";     break;
        case RunMode_Operator:      modeStr = "Operator"; break;
    }
    LCD_setText(1, LCDTextOptions_None, "%-8s%6d s", modeStr, (int) (lastTime / 1000));
    LCD_setText(2, LCDTextOptions_None, "<VexOS>%6.0f Hz", (1000.0 * LOOP_SAMPLE / loopTime));
}

// handle the battery LCD menu //
static void drawLCDBatteryScreen(LCDScreen* screen, LCDButtonType pressed) {
    if(expanderPort != 0) {
        LCD_setText(1, LCDTextOptions_Centered, "Main Expd Back");
        LCD_setText(2, LCDTextOptions_Centered, "%1.1fV %1.1fV %1.1fV", GetMainBattery(),
                    /*GetExpanderBattery()*/0.0, GetBackupBattery());
    } else {
        LCD_setText(1, LCDTextOptions_Centered, "Main Back");
        LCD_setText(2, LCDTextOptions_Centered, "%1.1fV %1.1fV", GetMainBattery(),
                    GetBackupBattery());
    }
}

// handle the autonomous LCD window //
static bool getLCDAutonomousStatus(LCDScreen* screen) {
    return autonomousPrograms.nodeCount > 0;
}

static void drawLCDAutonomousScreen(LCDScreen* screen, LCDButtonType pressed) {
    LCD_setText(1, LCDTextOptions_Centered, "Auto Program");
    
    // move the selection //
    if(pressed == LCDButtonType_Right) {
        if(activeAutonomous == NULL) {
            setActiveAutonomous(autonomousPrograms.firstNode);
        } else if(activeAutonomous->next != NULL) {
            setActiveAutonomous(activeAutonomous->next);
        }
        PrintToScreen("right\n");
    } else if(pressed == LCDButtonType_Left) {
        if(activeAutonomous != NULL) {
            setActiveAutonomous(activeAutonomous->prev);
        }
        PrintToScreen("left\n");
    }
    
    // print the program choice to LCD //
    LCDTextOptions opts = LCDTextOptions_Centered;
    if(activeAutonomous == NULL) {
        if(autonomousPrograms.nodeCount > 0) opts |= LCDTextOptions_RightArrow;
        LCD_setText(2, opts, "(none)");
    } else {
        opts |= LCDTextOptions_LeftArrow;
        if(activeAutonomous->next != NULL) opts |= LCDTextOptions_RightArrow;
        Command* cmd = activeAutonomous->data;
        LCD_setText(2, opts, Command_getName(cmd));
    }
}

// handle the dashboard LCD menu //
static void drawLCDDashboardScreen(LCDScreen* screen, LCDButtonType pressed) {
    LCD_setText(1, LCDTextOptions_Centered, "Dashboard");
    bool dash = Dashboard_isEnabled();
    // change mode //
    if(!dash && pressed == LCDButtonType_Right) {
        Dashboard_setEnabled(dash = true);
    } else if(dash && pressed == LCDButtonType_Left) {
        Dashboard_setEnabled(dash = false);
    }
    // print mode //
    if(!dash) {
        LCD_setText(2, LCDTextOptions_Centered | LCDTextOptions_RightArrow, "Off");
    } else {
        LCD_setText(2, LCDTextOptions_Centered | LCDTextOptions_LeftArrow, "On");
    }
}

// initialize the LCD menus //
static void initializeLCD() {
    LCD_addScreen(LCDScreen_new("VexOS Title", &drawLCDTitleScreen, NULL));
    LCD_addScreen(LCDScreen_new("Battery", &drawLCDBatteryScreen, NULL));
    LCD_addScreen(LCDScreen_new("Autonomous", &drawLCDAutonomousScreen, &getLCDAutonomousStatus));
    LCD_addScreen(LCDScreen_new("Dashboard", &drawLCDDashboardScreen, NULL));
    LCD_addScreen(DebugValue_getLCDScreen());
    lcdInitialized = true;
    LCD_restoreHandler();
}

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
    
    // LCD updates //
    if(time > nextLCDTime) {
        if(!lcdInitialized) initializeLCD();
        if(LCD_isEnabled()) {
            LCD_run();
            nextLCDTime = time + LCD_PERIOD;
        }
    }
    
    // dashboard updates //
    if(time > nextDashTime) {
        if(!dashboardInitialized) initializeDashboard();
        if(Dashboard_isEnabled()) {
            Dashboard_update();
            nextDashTime = time + DASH_PERIOD;
        }
    }
}

void fireEvent(EventType type) {
    if(events[type] == NULL) return;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        ((EventHandler*) node->data)(type);
        node = node->next;
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

// internal operating system state //
//bool VOS_Error;
//ErrorLevel VexOS_ErrorLevel;

RunMode VexOS_getRunMode() {
    return runMode;
}

unsigned long VexOS_getRunTime() {
    return (unsigned long) (lastTime / 1000);
}

double VexOS_getLoopFrequency() {
    return (1000.0 * LOOP_SAMPLE / loopTime);
}

void VexOS_setProgramName(const char* name) {
    programName = name;
}

const char* VexOS_getProgramName() {
    return programName;
}

/*void VOS_SetPowerExpander(AnalogPort port, PowerExpanderType type) {
    expanderPort = port;
    expanderType = type;
}

AnalogPort VOS_GetExpanderPort() {
    return expanderPort;
}

PowerExpanderType VOS_GetExpanderType() {
    return expanderType;
}

float VOS_GetExpanderBattery() {
    if(!expanderPort || !expanderType) return 0.0f;
    return ((GetAnalogInput(expanderPort) * 10.0f) / expanderType);
}*/

bool VexOS_addEventHandler(EventType type, EventHandler* handler) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_OperatorPeriodic, VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);
    // check if already there //
    if(VexOS_hasEventHandler(type, handler)) return false;
    // add list if not there //
    if(events[type] == NULL) {
        events[type] = List_new();
    }
    // add handler to list //
    List_insertLast(events[type], List_newNode(handler));
    return true;
}

bool VexOS_removeEventHandler(EventType type, EventHandler* handler) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_OperatorPeriodic, VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);
    
    if(events[type] == NULL) return false;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        if(((EventHandler*) node->data) == handler) {
            List_remove(node);
            free(node);
            return true;
        }
        node = node->next;
    }
    return false;
}

bool VexOS_hasEventHandler(EventType type, EventHandler* handler) {
    ErrorMsgIf(type < EventType_DisabledStart || type > EventType_OperatorPeriodic, VEXOS_ARGRANGE, "Invalid EventType");
    ErrorIf(handler == NULL, VEXOS_ARGNULL);
    
    if(events[type] == NULL) return false;
    ListNode* node = events[type]->firstNode;
    while(node != NULL) {
        if(((EventHandler*) node->data) == handler) return true;
        node = node->next;
    }
    return false;
}

unsigned int VexOS_addAutonomous(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    if(VexOS_hasAutonomous(cmd)) return 0;
    List_insertLast(&autonomousPrograms, List_newNode(cmd));
    autoChanged = true;
    return autonomousPrograms.nodeCount;
}

bool VexOS_removeAutonomous(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    if(!VexOS_hasAutonomous(cmd)) return false;
    ListNode* node = List_findNode(&autonomousPrograms, cmd);
    if(node != NULL) {
        if(activeAutonomous == node) {
            setActiveAutonomous(NULL);
        }
        List_remove(node);
        free(node);
        return true;
    }
    return false;
}

bool VexOS_hasAutonomous(Command* cmd) {
    ErrorIf(cmd == NULL, VEXOS_ARGNULL);
    
    return (List_findNode(&autonomousPrograms, cmd) != NULL);
}

unsigned int VexOS_getAutonomousCount() {
    return autonomousPrograms.nodeCount;
}

// starting at 1 //
Command* VexOS_getAutonomousByNumber(unsigned int n) {
    ErrorIf(n < 1 || n > VexOS_getAutonomousCount(), VEXOS_ARGRANGE);
    
    ListNode* node = List_getByIndex(&autonomousPrograms, n - 1);
    return (node != NULL)? node->data: NULL;
}

Command* VexOS_getSelectedAutonomous() {
    if(activeAutonomous == NULL) return NULL;
    return (Command*) activeAutonomous->data;
}

void VexOS_setSelectedAutonomous(Command* cmd) {
    if(cmd == NULL) activeAutonomous = NULL;
    ListNode* active = List_findNode(&autonomousPrograms, cmd);
    if(active == NULL) {
        VexOS_addAutonomous(cmd);
        active = autonomousPrograms.lastNode; // the newly added node //
        return;
    }
    setActiveAutonomous(active);
}

void VexOS_setSelectedAutonomousByNumber(unsigned int n) {
    ErrorIf(n > VexOS_getAutonomousCount(), VEXOS_ARGRANGE);
    
    VexOS_setSelectedAutonomous(VexOS_getAutonomousByNumber(n));
}

/********************************************************************
 * easyC Entry Points                                               *
 ********************************************************************/

// main entry point from easyC, always called //
void VexOS_Initialize() {
    runMode   = RunMode_VexOS_Setup;
    Info("VexOS Initialize\n\n");
    
    ErrorCode err;
    switch(err = setjmp(ErrorEvn)) {
        case VEXOS_NOERROR:
            // call robot initializer //
            runMode = RunMode_Initialize;
            InitializeRobot();
            restoreActiveAutonomous();
            // main run loop //
            loopStart();
            fireEvent(EventType_DisabledStart);
            while(!IsEnabled()) {
                loopPeriodic();
                fireEvent(EventType_DisabledPeriodic);
            }
            fireEvent(EventType_DisabledEnd);
            break;
        default:
            // an error occurred //
            VexOS_failWithError(err);
    }
}

void VexOS_OperatorControl(unsigned long ulTime) {
    runMode = RunMode_Operator;
    Info("OperatorControl\n");
    
    ErrorCode err;
    switch(err = setjmp(ErrorEvn)) {
        case VEXOS_NOERROR:
            // main loop //
            loopStart();
            fireEvent(EventType_OperatorStart);
            while(IsEnabled()) {
                loopPeriodic();
                fireEvent(EventType_OperatorPeriodic);
            }
        default:
            // an error occurred //
            VexOS_failWithError(err);
    }
}

void VexOS_Autonomous(unsigned long ulTime) {
    runMode = RunMode_Autonomous;
    Info("Autonomous\n");
    
    ErrorCode err;
    switch(err = setjmp(ErrorEvn)) {
        case VEXOS_NOERROR:
            // main loop //
            loopStart();
            fireEvent(EventType_AutonomousStart);
            // check for autonomous program and start if present //
            Command* autop = VexOS_getSelectedAutonomous();
            if(autop != NULL) {
                Command_start(autop);
            }
            while(IsEnabled()) {
                loopPeriodic();
                fireEvent(EventType_AutonomousPeriodic);
            }
        default:
            // an error occurred //
            VexOS_failWithError(err);
    }
}
