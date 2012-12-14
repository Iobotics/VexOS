//
//  MotorGroup.c
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/13/2012.
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

#include "Hardware.h"
#include "Device.h"
#include "Motor.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct MotorGroup {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    List*         children;
    Power         power;
};

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

MotorGroup* MotorGroup_new(String name) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    MotorGroup* ret = malloc(sizeof(MotorGroup));
    ret->type     = DeviceType_MotorGroup;
    ret->name     = name;
    ret->children = List_new();
    ret->power    = 0.0;
    Device_addVirtualDevice((Device*) ret);
    return ret;
}

MotorGroup* MotorGroup_delete(MotorGroup* group) {
    if(group) {
        Device_remove((Device*) group);
        // free children //
        ListNode* node = group->children->firstNode;
        while(node != NULL) {
            ListNode* temp = node->next;
            MotorGroup_removeMotor(group, node->data);
            free(node);
            node = temp;
        }
        free(group->children);
        free(group);
    }
    return NULL;
}

void MotorGroup_addMotor(MotorGroup* group, Motor* motor) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(motor == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(Motor_getGroup(motor), VEXOS_OPINVALID, 
               "Motor is already part of a MotorGroup: %s", motor->name);

    Motor_setGroup(motor, group);
    List_insertLast(group->children, List_newNode(motor));
}

void MotorGroup_removeMotor(MotorGroup* group, Motor* motor) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(motor == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(Motor_getGroup(motor) != group, VEXOS_OPINVALID, 
               "Motor is not part of MotorGroup: %s", motor->name);

    Motor_setGroup(motor, NULL);
    ListNode* node = List_findNode(group->children, motor);
    if(node) List_remove(node);
}

const List* MotorGroup_getMotorList(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->children;
}

Power MotorGroup_get(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->power;
}

void MotorGroup_set(MotorGroup* group, Power power) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    ListNode* node = group->children->firstNode;
    while(node != NULL) {
        Motor_set(node->data, power); 
        node = node->next;
    }
}
