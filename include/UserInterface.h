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

Dashboard*  Dashboard_new(String name);
Dashboard*  Dashboard_delete(Dashboard* dash);
void        Dashboard_addWindow(Dashboard* dash, Window* win);
void        Dashboard_addWindowWithPosition(Dashboard* dash, Window* win, char x, char y);
const List* Dashboard_getWindowList(Dashboard* dash);
void        Dashboard_removeWindow(Dashboard* dash, Window* win);
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

typedef void (WindowDrawCallback)(Window* win, bool fullRefresh);

Window*    Window_new(String name, WindowDrawCallback* callback);
Window*    Window_delete(Window* win);
String     Window_getName(Window* win);
Dashboard* Window_getDashboard(Window* win);
void       Window_setSize(Window* win, char width, char height);
char       Window_getWidth(Window* win);
char       Window_getHeight(Window* win);
void       Window_setPosition(Window* win, char x, char y);
Point      Window_getPosition(Window* win);
char       Window_getLeft(Window* win);
char       Window_getTop(Window* win);
Rect       Window_getInnerRect(Window* win);
Rect       Window_getOuterRect(Window* win);

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

void LCD_addScreen(LCD* lcd, LCDScreen* screen);
void LCD_removeScreen(LCD* lcd, LCDScreen* screen);
void LCD_setText(LCD* lcd, unsigned char line, LCDTextOptions opts, String fmtString, ...);
bool LCD_restoreLastScreen(LCD* lcd);

/********************************************************************
 * Public API: LCDScreen                                            *
 ********************************************************************/

typedef enum {
    LCDButtonType_Left   = 0x01,
    LCDButtonType_Center = 0x02,
    LCDButtonType_Right  = 0x04
} LCDButtonType;

typedef void (LCDDrawCallback)(LCDScreen* screen, LCDButtonType buttons);
typedef bool (LCDStatusCallback)(LCDScreen* screen);

LCDScreen* LCDScreen_new(String name, LCDDrawCallback* drawCallback, LCDStatusCallback* statusCallback);
LCDScreen* LCDScreen_delete(LCDScreen* screen);
String     LCDScreen_getName(LCDScreen* screen);
LCD*       LCDScreen_getLCD(LCDScreen* screen);

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

Window* Device_getWindow(DeviceWindowType type);

