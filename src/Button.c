//
//  Button.c
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

#include "Button.h"
#include "ButtonClass.h"
#include "Scheduler.h"
#include "Error.h"

/********************************************************************
 * ButtonScheduler Structure (inner class)                          *
 ********************************************************************/

typedef enum  {
    ButtonSchedulerType_WhenPressed,
    ButtonSchedulerType_WhileHeld,
    ButtonSchedulerType_WhenReleased,
    ButtonSchedulerType_WhileToggled
} ButtonSchedulerType;

struct ButtonScheduler {
    Button*             button;
    Command*            command;
    ButtonSchedulerType type;
    bool                pressedLast;
};

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

typedef struct {
    unsigned int id;
    List* list;
} ToggleGroup;

static unsigned int lastObjectId = 0;

static List toggleGroups;

static ToggleGroup* findToggleGroup(Button* button) {
    if(button == NULL) return NULL;
    ListNode* node = toggleGroups.firstNode;
    while(node != NULL) {
        ToggleGroup* group = (ToggleGroup*) node->data;
        ListNode* xnode    = group->list->firstNode;
        while(xnode != NULL) {
            if(((Button*) xnode->data) == button) return group;
            xnode = xnode->next;
        }
        node = node->next;
    }
    return NULL;
}

static ToggleGroup* getToggleGroup(unsigned int id) {
    ListNode* node = toggleGroups.firstNode;
    ToggleGroup* group = NULL;
    while(node != NULL) {
        group = (ToggleGroup*) node->data;
        if(group->id == id) return group;
        node = node->next;
    }
    // create a new one //
    group = malloc(sizeof(ToggleGroup));
    group->id   = id;
    group->list = List_new();
    List_insertLast(&toggleGroups, List_newNode(group));
    return group;
}

static ButtonScheduler* getScheduler(Button* button, Command* cmd, ButtonSchedulerType type) {
    ButtonScheduler* sched = malloc(sizeof(ButtonScheduler));
    sched->button      = button;
    sched->command     = cmd;
    sched->type        = type;
    sched->pressedLast = Button_get(button);
    return sched;
}

static void callConstructorMethod(Button* button, void (method)(va_list), va_list argp) {
    if(!method) return;
    Button* old = *(button->class->selfPtr);
    *(button->class->selfPtr) = button;
    method(argp);
    *(button->class->selfPtr) = old;
}

static void callVoidMethod(Button* button, void (method)()) {
    if(!method) return;
    Button* old = *(button->class->selfPtr);
    *(button->class->selfPtr) = button;
    method();
    *(button->class->selfPtr) = old;
}

static bool callBoolMethod(Button* button, bool (method)()) {
    if(!method) return true;
    Button* old = *(button->class->selfPtr);
    *(button->class->selfPtr) = button;
    bool ret = method();
    *(button->class->selfPtr) = old;
    return ret;
}

static void initializeButtonClass(ButtonClass* class) {
    class->initialized = true;
    lastObjectId = 0;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Button_setvName(Button* button, String fmt, va_list argp) {
    char* old = (char*) button->name;
    vasprintf((char**) &button->name, fmt, argp);
    if(old) free(old);
}

void Button_setName(Button* button, String fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    Button_setvName(button, fmt, argp);
    va_end(argp);
}

void Button_setvArgs(Button* button, String fmt, va_list argp) {
    char* old = (char*) Button_getName(button);
    char* xfmt;
    asprintf(&xfmt, "%s(%s)", old, fmt);
    // print new name //
    vasprintf((char**) &button->name, xfmt, argp);
    free(xfmt);
    free(old);
}

void Button_setArgs(Button* button, String fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    Button_setvArgs(button, fmt, argp);
    va_end(argp);
}

void Button_executeScheduler(ButtonScheduler* sched) {
    // get the button //
    bool state = Button_get(sched->button);
    // implement appropriate algorithm //
    switch(sched->type) {
        case ButtonSchedulerType_WhenPressed:
            if(state) {
                if(!sched->pressedLast) {
                    sched->pressedLast = true;
                    Command_start(sched->command);
                }
            } else {
                sched->pressedLast = false;
            }
            break;
        case ButtonSchedulerType_WhileHeld:
            if(state) {
                sched->pressedLast = true;
                Command_start(sched->command);
            } else {
                if(sched->pressedLast) {
                    sched->pressedLast = false;
                    Command_cancel(sched->command);
                }
            }
            break;
        case ButtonSchedulerType_WhenReleased:
            if(state) {
                sched->pressedLast = true;
            } else {
                if(sched->pressedLast) {
                    sched->pressedLast = false;
                    Command_start(sched->command);
                }
            }
            break;
        case ButtonSchedulerType_WhileToggled:
            if(state) {
                if(!sched->pressedLast) {
                    sched->pressedLast = true;
                    sched->button->toggleState ^= true;
                    // if toggled on, clear the rest of the group //
                    if(sched->button->toggleState) {
                        ToggleGroup* group = findToggleGroup(sched->button);
                        if(group != NULL) {
                            ListNode* gnode = group->list->firstNode;
                            while(gnode != NULL) {
                                Button* gbutton = (Button*) gnode->data;
                                if(gbutton != sched->button) {
                                    gbutton->toggleState = false;
                                }
                                gnode = gnode->next;
                            }
                        }
                    }
                }
            } else {
                sched->pressedLast = false;
            }
            if(sched->button->toggleState) {
                Command_start(sched->command);
            }
            break;
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Button* Button_new(ButtonClass* class, ...) {
    // check the class //
    if(!class->initialized) {
        initializeButtonClass(class);
    }
    // create the Button //
    Button* button = malloc(sizeof(Button));
    button->class       = class;
    button->objectId    = ++lastObjectId;
    button->instanceId  = ++class->lastInstanceId;
    button->name        = NULL;
    button->toggleState = false;
    // build the fields //
    button->fields = (class->fieldSize > 0)? malloc(class->fieldSize): NULL;
    // call the constructor //
    va_list argp;
    va_start(argp, class);
    callConstructorMethod(button, class->constructor, argp);
    va_end(argp);
    return button;
}

Button* Button_delete(Button* button) {
    if(!button) return NULL;
    callVoidMethod(button, button->class->destructor);
    free(button);
    return NULL;
}

String Button_getName(Button* button) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    
    if(!button->name) {
        asprintf((char**) &button->name, "%s[%d]", button->class->name, button->instanceId);
    }
    return button->name;
}

bool Button_get(Button* button) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    
    // selector method //
    return callBoolMethod(button, button->class->get);
}

void Button_whenPressed(Button* button, Command* cmd) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    ErrorIf(cmd == NULL,    VEXOS_ARGNULL);
    
    Scheduler_addButtonScheduler(getScheduler(button, cmd, ButtonSchedulerType_WhenPressed));
}

void Button_whileHeld(Button* button, Command* cmd) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    ErrorIf(cmd == NULL,    VEXOS_ARGNULL);
    
    Scheduler_addButtonScheduler(getScheduler(button, cmd, ButtonSchedulerType_WhileHeld));
}

void Button_whenReleased(Button* button, Command* cmd) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    ErrorIf(cmd == NULL,    VEXOS_ARGNULL);
    
    Scheduler_addButtonScheduler(getScheduler(button, cmd, ButtonSchedulerType_WhenReleased));
}

void Button_whileToggled(Button* button, Command* cmd) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    ErrorIf(cmd == NULL,    VEXOS_ARGNULL);
    
    Scheduler_addButtonScheduler(getScheduler(button, cmd, ButtonSchedulerType_WhileToggled));
}

unsigned int Button_getToggleGroup(Button* button) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    
    ToggleGroup* group = findToggleGroup(button);
    return (group != NULL)? group->id: -1;
}

void Button_setToggleGroup(Button* button, unsigned int id) {
    ErrorIf(button == NULL, VEXOS_ARGNULL);
    
    ToggleGroup* group = findToggleGroup(button);
    ListNode* tnode = NULL;
    if(group != NULL) {
        if(group->id == id) return;
        tnode = List_findNode(group->list, button);
    } else {
        tnode = List_newNode(button);
    }
    // add to group //
    List_insertLast(getToggleGroup(id)->list, tnode);
}

