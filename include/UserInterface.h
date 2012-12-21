//
//  UserInterface.h
//  VexOS for Vex Cortex, User Interface Layer
//
//  Created by Jeff Malins on 12/14/2012.
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

#include "VexOS.h"
#include "Hardware.h"

/********************************************************************
 * Public Object Definitions                                        *
 ********************************************************************/

typedef struct Dashboard Dashboard;
typedef struct Window    Window;
#ifndef _Hardware_h
    // already defined in the hardware file //
    typedef struct LCD LCD;
#endif
typedef struct LCDScreen LCDScreen;

/********************************************************************
 * Public API: Dashboard                                            *
 ********************************************************************/

#define DASH_PERIOD     100

Dashboard*  Dashboard_new(String);
Dashboard*  Dashboard_delete(Dashboard*);
void        Dashboard_addWindow(Dashboard*, Window*);
void        Dashboard_addWindowWithPosition(Dashboard*, Window*, char, char);
const List* Dashboard_getWindowList(Dashboard*);
void        Dashboard_removeWindow(Dashboard*, Window*);
const List* Dashboard_getList();
void        Dashboard_refresh();
LCDScreen*  Dashboard_getLCDScreen();
bool        Dashboard_restoreLast();

/********************************************************************
 * Public API: Window                                               *
 ********************************************************************/

// colors used in display //
typedef enum {
    Color_Black       = 0x000000,
    Color_Red         = 0x0000FF,
    Color_Green       = 0x00FF00,
    Color_Blue        = 0xFF0000,
    Color_Yellow      = 0x00FFFF,
    Color_Cyan        = 0xFFFF00,
    Color_Magenta     = 0xFF00FF,
    Color_White       = 0xFFFFFF,
    Color_DarkRed     = 0x000080,
    Color_DarkGreen   = 0x008000,
    Color_DarkBlue    = 0x800000,
    Color_DarkYellow  = 0x008080,
    Color_DarkCyan    = 0x808000,
    Color_DarkMagenta = 0x800080,
    Color_Grey        = 0x808080
} Color;

typedef struct {
    unsigned char left, top, right, bottom;
} Rect;

typedef struct {
    unsigned char x, y;
} Point;

typedef void (WindowDrawCallback)(Window*, bool);

Window*    Window_new(String, WindowDrawCallback*);
Window*    Window_delete(Window*);
String     Window_getName(Window*);
Dashboard* Window_getDashboard(Window*);
void       Window_setSize(Window*, char, char);
char       Window_getWidth(Window*);
char       Window_getHeight(Window*);
void       Window_setPosition(Window*, char, char);
Point      Window_getPosition(Window*);
char       Window_getLeft(Window*);
char       Window_getTop(Window*);
Rect       Window_getInnerRect(Window*);
Rect       Window_getOuterRect(Window*);

/********************************************************************
 * Public API: LCD                                                  *
 ********************************************************************/

#define LCD_PERIOD      100

typedef enum {
    LCDTextOptions_None         = 0x00,
    LCDTextOptions_Centered     = 0x01,
    LCDTextOptions_LeftArrow    = 0x02,
    LCDTextOptions_RightArrow   = 0x04
} LCDTextOptions;

void LCD_addScreen(LCD*, LCDScreen*);
void LCD_removeScreen(LCD*, LCDScreen*);
void LCD_setText(LCD*, unsigned char, LCDTextOptions, String, ...);
bool LCD_restoreLastScreen(LCD*);

/********************************************************************
 * Public API: LCDScreen                                            *
 ********************************************************************/

typedef enum {
    LCDButtonType_Left   = 0x01,
    LCDButtonType_Center = 0x02,
    LCDButtonType_Right  = 0x04
} LCDButtonType;

typedef void (LCDDrawCallback)(LCDScreen*, LCDButtonType);
typedef bool (LCDStatusCallback)(LCDScreen*);

LCDScreen* LCDScreen_new(String name, LCDDrawCallback*, LCDStatusCallback*);
LCDScreen* LCDScreen_delete(LCDScreen*);
String     LCDScreen_getName(LCDScreen*);
LCD*       LCDScreen_getLCD(LCDScreen*);

/********************************************************************
 * Public API: Autonomous (UI Hook)                                 *
 ********************************************************************/

Window*     Autonomous_getWindow();
LCDScreen*  Autonomous_getLCDScreen();

/********************************************************************
 * Public API: Battery (UI Hook)                                    *
 ********************************************************************/

Window*     Battery_getWindow();
LCDScreen*  Battery_getLCDScreen();

/********************************************************************
 * Public API: Scheduler (UI Hook)                                  *
 ********************************************************************/

Window* Scheduler_getWindow();

/********************************************************************
 * Public API: Status (UI Hook)                                     *
 ********************************************************************/

Window*     Status_getWindow();
LCDScreen*  Status_getLCDScreen();

/********************************************************************
 * Public API: DebugValue (UI Hook)                                 *
 ********************************************************************/

Window*     DebugValue_getWindow();
LCDScreen*  DebugValue_getLCDScreen();

/********************************************************************
 * Public API: Device (UI Hook)                                     *
 ********************************************************************/

#define DEVICE_WINDOW_COUNT     5

typedef enum {
    DeviceWindowType_Digital,
    DeviceWindowType_Analog,
    DeviceWindowType_PWM,
    DeviceWindowType_UART
} DeviceWindowType;

Window* Device_getWindow(DeviceWindowType);

