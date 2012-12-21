//
//  Battery.c
//  VexOS for Vex Cortex
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

#include "API.h"

#include "UserInterface.h"
#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static float thresholdMainG   = BatteryThreshold_MAIN_G;
static float thresholdMainY   = BatteryThreshold_MAIN_Y;
static float thresholdBackupG = BatteryThreshold_BACKUP_G;
static float thresholdBackupY = BatteryThreshold_BACKUP_Y;

static Color getBatteryColor(float volts, bool backup) {
    if(backup) {
        if(volts >= thresholdMainG) return Color_DarkGreen;
        if(volts >= thresholdMainY) return Color_DarkYellow;
        return Color_Red;
    } else {
        if(volts >= thresholdBackupG) return Color_DarkGreen;
        if(volts >= thresholdBackupY) return Color_DarkYellow;
        return Color_Red;
    }
    return Color_Red;
}

static PowerExpander* getMainPowerExpander() {
    static PowerExpander* expand  = NULL;
    static bool           checked = false;
    if(!checked) {
        expand = (PowerExpander*) Device_getByType(DeviceType_PowerExpander);
        //PrintToScreen("type: %d\n", PowerExpander_getType(expand));
        checked = true;
    }
    return expand;
}

static void updateWindow(Window* win, bool full) {
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left = innerRect.left;
    unsigned char top  = innerRect.top;
    float volts;

    volts = GetMainBattery();
    PrintTextToGD(top, left, Color_Black, "Main:\n");
    PrintTextToGD(top++, left + 10, getBatteryColor(volts, false), "%1.2f V\n", volts);
    PowerExpander* expand = getMainPowerExpander();
    if(expand) {
        volts = PowerExpander_getBatteryVoltage(expand);
        //PrintToScreen("type2: %d\n", PowerExpander_getType(expand));
        PrintTextToGD(top,   left, Color_Black, "Expander:\n");
        PrintTextToGD(top++, left + 10, getBatteryColor(volts, false), "%1.2f V\n", volts);
    }
    volts = GetBackupBattery();
    PrintTextToGD(top,   left, Color_Black, "Backup:\n");
    PrintTextToGD(top++, left + 10, getBatteryColor(volts, true), "%1.2f V\n", volts);
}

static void drawLCDScreen(LCDScreen* screen, LCDButtonType pressed) {
    LCD* lcd = LCDScreen_getLCD(screen);
    PowerExpander* expand = getMainPowerExpander();
    if(expand) {
        LCD_setText(lcd, 1, LCDTextOptions_Centered, "Main Expd Back");
        LCD_setText(lcd, 2, LCDTextOptions_Centered, "%1.1fV %1.1fV %1.1fV", GetMainBattery(),
                    PowerExpander_getBatteryVoltage(expand), GetBackupBattery());
    } else {
        LCD_setText(lcd, 1, LCDTextOptions_Centered, "Main Back");
        LCD_setText(lcd, 2, LCDTextOptions_Centered, "%1.1fV %1.1fV", GetMainBattery(),
                    GetBackupBattery());
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void Battery_setMainThresholds(float threshG, float threshY) {
    thresholdMainG = threshG;
    thresholdMainY = threshY;
}

void Battery_getMainThresholds(float* threshG, float* threshY) {
    *threshG = thresholdMainG;
    *threshY = thresholdMainY;
}

void Battery_setBackupThresholds(float threshG, float threshY) {
    thresholdBackupG = threshG;
    thresholdBackupY = threshY;
}

void Battery_getBackupThresholds(float* threshG, float* threshY) {
    *threshG = thresholdBackupG;
    *threshY = thresholdBackupY;
}

/********************************************************************
 * Public API (UI Hooks)                                            *
 ********************************************************************/

Window* Battery_getWindow() {
    static Window* window = NULL;
    if(window) return window;
    window = Window_new("Battery", &updateWindow);
    Window_setSize(window, 16, 3);
    return window;
}

LCDScreen* Battery_getLCDScreen() {
    static LCDScreen* screen = NULL;
    if(screen) return screen;
    screen = LCDScreen_new("Battery", &drawLCDScreen, NULL);
    return screen;
}

