//
//  LCD.c
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
#include "Hardware.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define BUTTON_COUNT    3
#define LCD_CHARS       16
#define LCD_LINES       2

static UARTPort     lcdUartPort;
static bool         lcdBacklight;
static List         screens;
static ListNode*    currentScreen;

static LCDButtonType getButtons() {
    static const LCDButtonType buttonMap[BUTTON_COUNT] = {
        LCDButtonType_Left, LCDButtonType_Center, LCDButtonType_Right
    };
    static unsigned char lastState[BUTTON_COUNT];
    unsigned char button[BUTTON_COUNT];
    
    // check the button and debounce //
    LCDButtonType pressed = 0;
    GetLCDButtonsWatcher(lcdUartPort, &button[0], &button[1], &button[2]);
    int i;
    for(i = 0; i < BUTTON_COUNT; i++) {
        if(button[i]) {
            if(!lastState[i]) {
                pressed      = buttonMap[i];
                lastState[i] = 1;
            }
        } else {
            lastState[i] = 0;
        }
    }
    return pressed;
}

static void setScreenNode(ListNode* node) {
    currentScreen = node;
    GlobalData(GLOBALDATA_LCD_MENU) = (node)? (List_indexOfNode(node) + 1): 0;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void LCD_run() {
    if(screens.nodeCount == 0) return;
    if(currentScreen == NULL) {
        setScreenNode(screens.firstNode);
    }
    
    // pass buttons to handler //
    LCDButtonType pressed = getButtons();
    bool advance = true;
    int tries = 0;
    while(advance && tries < screens.nodeCount) {
        LCDScreen* screen = currentScreen->data;
        if(LCDScreen_isDisplayed(screen)) {
            LCDScreen_draw(screen, pressed);
            advance = false;
        }
        // advance to next item //
        if(advance || pressed == LCDButtonType_Center) {
            if(currentScreen->next != NULL) {
                setScreenNode(currentScreen->next);
            } else {
                setScreenNode(currentScreen->list->firstNode);
            }
        }
        // prevent infinite loop when all menus want to advance //
        tries++;
    }
}

void LCD_setText(unsigned char line, LCDTextOptions opts, String text, ...) {
    ErrorIf(line < 1 || line > LCD_LINES, VEXOS_ARGRANGE);
    
    if(lcdUartPort == 0) return;
    char buffer[LCD_CHARS + 1];
    
    // create the formatted string to allocated memory //
    char* ntext;
    va_list argp;
    va_start(argp, text);
    vasprintf(&ntext, text, argp);
    va_end(argp);
    
    // prepare the buffer //
    memset(&buffer, ' ', LCD_CHARS);
    buffer[LCD_CHARS] = '\0';
    size_t maxLen = LCD_CHARS;
    size_t start  = 0;
    if(opts & LCDTextOptions_LeftArrow) {
        maxLen--;
        buffer[0] = '\xF7';
        start++;
    }
    if(opts & LCDTextOptions_RightArrow) {
        maxLen--;
        buffer[LCD_CHARS - 1] = '\xF6';
    }
    
    size_t strLen = strlen(ntext);
    if(strLen > maxLen) {
        memcpy(&buffer[start], ntext, maxLen);
    } else if(opts & LCDTextOptions_Centered) {
        start += (maxLen - strLen) / 2;
        memcpy(&buffer[start], ntext, strLen);
    } else {
        memcpy(&buffer[start], ntext, strLen);
    }
    SetLCDText(lcdUartPort, line, (const char*) &buffer);
    free(ntext);
}

bool LCD_restoreHandler() {
    unsigned int stored = (unsigned int) GlobalData(GLOBALDATA_LCD_MENU);
    if(stored > 0) {
        currentScreen = List_getByIndex(&screens, stored - 1);
        return true;
    }
    return false;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void LCD_setPort(UARTPort uart) {
    ErrorIf(uart < UARTPort_1 || uart > UARTPort_2, VEXOS_ARGRANGE);
    
    if(uart == lcdUartPort) return;
    if(!uart && lcdUartPort != 0) {
        StopLCDButtonsWatcher(lcdUartPort);
    }
    // start LCD //
    lcdUartPort = uart;
    InitLCD(lcdUartPort);
    StartLCDButtonsWatcher(lcdUartPort);
    LCD_setBacklight(true);
}

UARTPort LCD_getPort() {
    return lcdUartPort;
}

bool LCD_isEnabled() {
    return (lcdUartPort != 0);
}

void LCD_setBacklight(bool light) {
    if(lcdUartPort == 0) return;
    SetLCDLight(lcdUartPort, light);
    lcdBacklight = light;
}

bool LCD_getBacklight() {
    return lcdBacklight;
}

void LCD_addScreen(LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&screens, screen);
    if(node != NULL) return;
    List_insertLast(&screens, List_newNode(screen));
}

void LCD_removeScreen(LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&screens, screen);
    if(node == NULL) return;
    setScreenNode(List_remove(node));
}
