//
//  UniIntake.c
//  VexOS for Vex Cortex, Universal Robot Library
//
//  Created by Jeff Malins on 12/29/2012.
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

#include "Subsystem.h"
#include "UniBot.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineSubsystem(UniIntake);

typedef struct {
    int    id;
    String name;
    Power  power;
} Direction;

static UniIntakeSetup setup;
List      directionList;
ListNode* directionNode;

static void constructor() {
    // call the end-user constructor //
    UniIntake_configure(self, &setup);
}

static void initialize() {
    ErrorMsgIf(!setup.motors, VEXOS_ARGNULL, "Intake motors are not defined");

    setDefaultCommand(UniIntake_getDefaultCommand(self));
}

static void setDirectionNode(ListNode* node) {
    if(node) {
        Direction* direction = node->data;
        MotorGroup_setPower(setup.motors, direction->power);
    }
    directionNode = node;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void UniIntake_addDirection(int id, String name, Power power) {
    ErrorIf(id < 0, VEXOS_ARGRANGE);
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorIf(power < -1.0 || power > 1.0, VEXOS_ARGRANGE);
    
    // make sure id is not already defined //
    ListNode* node = directionList.firstNode;
    while(node != NULL) {
        Direction* direction = node->data;
        ErrorMsgIf(direction->id == id, VEXOS_ARGINVALID, "Direction ID is already defined: %d", id);
        node = node->next;
    }

    // create the direction and add it //
    Direction* direction = malloc(sizeof(Direction));
    direction->id    = id;
    direction->name  = name;
    direction->power = power;
    List_insertLast(&directionList, List_newNode(direction));
}

int UniIntake_getDirection() {
    if(!directionNode) return -1;
    Direction* direction = directionNode->data;
    return direction->id;
}

String UniIntake_getDirectionName() {
    if(!directionNode) return NULL;
    Direction* direction = directionNode->data;
    return direction->name;
}

Power UniIntake_getDirectionPower() {
    if(!directionNode) return 0.0;
    Direction* direction = directionNode->data;
    return direction->power;
}

void UniIntake_setPosition(int id) {
    ListNode* node = directionList.firstNode;
    while(node != NULL) {
        Direction* direction = node->data;
        if(direction->id == id) {
            setDirectionNode(node);
            return;
        }
        node = node->next;
    }
    RaiseError(VEXOS_ARGINVALID, "Invalid direction ID: %d", id);
}

void UniIntake_setPower(Power power) {
    setDirectionNode(NULL);
    MotorGroup_setPower(setup.motors, power);
}
