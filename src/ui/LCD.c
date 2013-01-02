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

#include "API.h"

#include "LCDScreen.h"
#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define BUTTON_COUNT    3
#define LCD_CHARS       16
#define LCD_LINES       2

struct LCD {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    Subsystem*    subsystem;
    // device item fields //
    UARTPort      port;
    bool          backlight;
    List          screens;
    ListNode*     currentScreen;
    unsigned char lastButtonState[BUTTON_COUNT];
};

static List lcds;

static void setScreenNode(LCD* lcd, ListNode* node) {
    lcd->currentScreen = node;
    if(lcd == (LCD*) lcds.firstNode->data) {
        GlobalData(GLOBALDATA_LCD_SCREEN) = (node)? (List_indexOfNode(node) + 1): 0;
    }
}

static LCDButtonType getButtons(LCD* lcd) {
    static const LCDButtonType buttonMap[BUTTON_COUNT] = {
        LCDButtonType_Left, LCDButtonType_Center, LCDButtonType_Right
    };
    unsigned char button[BUTTON_COUNT];
    
    // check the button and debounce //
    LCDButtonType pressed = 0;
    GetLCDButtonsWatcher(lcd->port, &button[0], &button[1], &button[2]);
    int i;
    for(i = 0; i < BUTTON_COUNT; i++) {
        if(button[i]) {
            if(!lcd->lastButtonState[i]) {
                pressed = buttonMap[i];
                lcd->lastButtonState[i] = 1;
            }
        } else {
            lcd->lastButtonState[i] = 0;
        }
    }
    return pressed;
}

static void runLCD(LCD* lcd) {
    if(lcd->screens.nodeCount == 0) return;
    if(lcd->currentScreen == NULL) {
        setScreenNode(lcd, lcd->screens.firstNode);
    }
    
    // pass buttons to handler //
    LCDButtonType pressed = getButtons(lcd);
    bool advance = true;
    int tries = 0;
    while(advance && tries < lcd->screens.nodeCount) {
        LCDScreen* screen = lcd->currentScreen->data;
        if(LCDScreen_isDisplayed(screen)) {
            LCDScreen_draw(screen, pressed);
            advance = false;
        }
        // advance to next item //
        if(advance || pressed == LCDButtonType_Center) {
            if(lcd->currentScreen->next != NULL) {
                setScreenNode(lcd, lcd->currentScreen->next);
            } else {
                setScreenNode(lcd, lcd->currentScreen->list->firstNode);
            }
        }
        // prevent infinite loop when all menus want to advance //
        tries++;
    }
}

static void eventCallback(EventType type, void* state) {
    static unsigned long nextLCDTime = 0;
    unsigned long time = GetMsClock();
    
    // normal display //
    if(time > nextLCDTime || type == EventType_SystemError) {
        ListNode* node = lcds.firstNode;
        while(node != NULL) {
            LCD* lcd = node->data;
            if(type != EventType_SystemError) {
                runLCD(lcd);
            } else {
                LCD_setText(lcd, 1, LCDTextOptions_Centered, "ERROR");
                LCD_setText(lcd, 2, LCDTextOptions_None, Error_getMessage());
            }
            node = node->next;
        }
        nextLCDTime = time + LCD_PERIOD;
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

LCD* LCD_new(String name, UARTPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    LCD* ret = malloc(sizeof(LCD));
    ret->type      = DeviceType_LCD;
    ret->name      = name;
    ret->port      = port;
    ret->backlight = false;
    memset(&ret->screens, 0, sizeof(List));
    ret->currentScreen = NULL;
    Device_addUART(port, (Device*) ret);
    List_insertLast(&lcds, List_newNode(ret));

    // start the LCD //
    InitLCD(ret->port);
    StartLCDButtonsWatcher(ret->port);
    LCD_setBacklight(ret, true);

    // check for first LCD and install handler //
    if(lcds.nodeCount == 1) {
        VexOS_addEventHandler(EventType_DisabledPeriodic,   &eventCallback, NULL);
        VexOS_addEventHandler(EventType_AutonomousPeriodic, &eventCallback, NULL);
        VexOS_addEventHandler(EventType_OperatorPeriodic,   &eventCallback, NULL);
        VexOS_addEventHandler(EventType_SystemError,        &eventCallback, NULL);
    }
    return ret;
}

UARTPort LCD_getPort(LCD* lcd) {
    ErrorIf(lcd == NULL, VEXOS_ARGNULL);

    return lcd->port;
}

void LCD_setBacklight(LCD* lcd, bool light) {
    ErrorIf(lcd == NULL, VEXOS_ARGNULL);

    SetLCDLight(lcd->port, light);
    lcd->backlight = light;
}

bool LCD_getBacklight(LCD* lcd) {
    ErrorIf(lcd == NULL, VEXOS_ARGNULL);

    return lcd->backlight;
}

void LCD_addScreen(LCD* lcd, LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&lcd->screens, screen);
    if(node != NULL) return;
    LCDScreen_setLCD(screen, lcd);
    List_insertLast(&lcd->screens, List_newNode(screen));
}

void LCD_removeScreen(LCD* lcd, LCDScreen* screen) {
    ErrorIf(screen == NULL, VEXOS_ARGNULL);
    
    ListNode* node = List_findNode(&lcd->screens, screen);
    if(node == NULL || lcd->currentScreen == node) return;
    LCDScreen_setLCD(screen, NULL);
    setScreenNode(lcd, List_remove(node));
}

void LCD_setText(LCD* lcd, unsigned char line, LCDTextOptions opts, String text, ...) {
    ErrorIf(line < 1 || line > LCD_LINES, VEXOS_ARGRANGE);
    
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
    SetLCDText(lcd->port, line, (const char*) &buffer);
    free(ntext);
}

bool LCD_restoreLastScreen(LCD* lcd) {
    // get the first LCD defined //
    ListNode* node = lcds.firstNode;
    LCD* firstLCD  = (node)? (LCD*) node->data: NULL;
    ErrorMsgIf(lcd != firstLCD, VEXOS_OPINVALID, "Restore is only allowed on the primary LCD");
    
    unsigned int stored = (unsigned int) GlobalData(GLOBALDATA_LCD_SCREEN);
    if(stored > 0) {
        lcd->currentScreen = List_getNodeByIndex(&lcd->screens, stored - 1);
        return true;
    }
    lcd->currentScreen = 0;
    return false;
}
