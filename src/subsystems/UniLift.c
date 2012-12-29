//
//  UniLift.c
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

#include <math.h>

#include "Subsystem.h"
#include "UniBot.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineSubsystem(UniLift);

typedef struct {
    int    id;
    String name;
    float  value;
} Position;

static UniLiftSetup setup;
static bool pidAllowed = false;
List        positionList;
ListNode*   positionNode;

static void constructor() {
    // set the default values //
    setup.type          = UniLiftType_Single;
    setup.gearRatio     = 1.0;
    setup.pid.kP        = UniLift_PID_Default_kP;
    setup.pid.kI        = UniLift_PID_Default_kI;
    setup.pid.kD        = UniLift_PID_Default_kD;

    // call the end-user constructor //
    UniLift_configure(self, &setup);
}

static void initialize() {
    // check the gearing //
    ErrorIf(setup.gearRatio <= 0, VEXOS_ARGINVALID);

    // validate we have the required motors //
    switch(setup.type) {
        case UniLiftType_Single:
            ErrorMsgIf(!setup.motors.single, VEXOS_ARGNULL, "Lift motor is not defined");
            MotorGroup_setOutputScaleFactor(setup.motors.single, setup.gearRatio);
            pidAllowed = (MotorGroup_getFeedbackType(setup.motors.single) != FeedbackType_None);
            if(pidAllowed) {
                MotorGroup_setPID(setup.motors.single, setup.pid.kP, setup.pid.kI, setup.pid.kD);
            }
            break;
        case UniLiftType_Split:
            ErrorMsgIf(!setup.motors.split.left || !setup.motors.split.right,
                       VEXOS_ARGNULL, "Split lift motors are not defined");
            MotorGroup_setOutputScaleFactor(setup.motors.split.left,  setup.gearRatio);
            MotorGroup_setOutputScaleFactor(setup.motors.split.right, setup.gearRatio);
            pidAllowed = (MotorGroup_getFeedbackType(setup.motors.split.left)  != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.split.right) != FeedbackType_None);
            if(pidAllowed) {
                MotorGroup_setPID(setup.motors.split.left,  setup.pid.kP, setup.pid.kI, setup.pid.kD);
                MotorGroup_setPID(setup.motors.split.right, setup.pid.kP, setup.pid.kI, setup.pid.kD);
            }
            break;
    }

    setDefaultCommand(UniLift_getDefaultCommand(self));
}

static void setPositionNode(ListNode* node) {
    if(node) {
        Position* position = node->data;
        switch(setup.type) {
            case UniLiftType_Single:
                MotorGroup_setSetpoint(setup.motors.single, position->value);
                if(positionNode == NULL) {
                    MotorGroup_setPIDEnabled(setup.motors.single, true);
                }
                break;
            case UniLiftType_Split:
                MotorGroup_setSetpoint(setup.motors.split.left,  position->value);
                MotorGroup_setSetpoint(setup.motors.split.right, position->value);
                if(positionNode == NULL) {
                    MotorGroup_setPIDEnabled(setup.motors.split.left,  true);
                    MotorGroup_setPIDEnabled(setup.motors.split.right, true);
                }
                break;
        }
    } else {
        // disabled PID //
        switch(setup.type) {
            case UniLiftType_Single:
                MotorGroup_setPIDEnabled(setup.motors.single, false);
                break;
            case UniLiftType_Split:
                MotorGroup_setPIDEnabled(setup.motors.split.left,  false);
                MotorGroup_setPIDEnabled(setup.motors.split.right, false);
                break;
        }
    }
    positionNode = node;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void UniLift_addPosition(int id, String name, float value) {
    ErrorMsgIf(!pidAllowed, VEXOS_OPINVALID, "UniLift does not have PID feedback");
    ErrorIf(id < 0, VEXOS_ARGRANGE);
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    
    // make sure id is not already defined //
    ListNode* node = positionList.firstNode;
    while(node != NULL) {
        Position* position = node->data;
        ErrorMsgIf(position->id == id, VEXOS_ARGINVALID, "Position ID is already defined: %d", id);
        node = node->next;
    }

    // create the position and add it //
    Position* position = malloc(sizeof(Position));
    position->id    = id;
    position->name  = name;
    position->value = value;
    List_insertLast(&positionList, List_newNode(position));
}

int UniLift_getPosition() {
    if(!positionNode) return -1;
    Position* position = positionNode->data;
    return position->id;
}

String UniLift_getPositionName() {
    if(!positionNode) return NULL;
    Position* position = positionNode->data;
    return position->name;
}

float UniLift_getPositionValue() {
    if(!positionNode) return NAN;
    Position* position = positionNode->data;
    return position->value;
}

void UniLift_setPosition(int id) {
    ErrorMsgIf(!pidAllowed, VEXOS_OPINVALID, "UniLift does not have PID feedback");

    ListNode* node = positionList.firstNode;
    while(node != NULL) {
        Position* position = node->data;
        if(position->id == id) {
            setPositionNode(node);
            return;
        }
        node = node->next;
    }
    RaiseError(VEXOS_ARGINVALID, "Invalid position ID: %d", id);
}

int UniLift_jogPosition(UniLiftJogDirection dir) {
    ErrorMsgIf(!pidAllowed, VEXOS_OPINVALID, "UniLift does not have PID feedback");

    ListNode* new = NULL;
    switch(dir) {
        case UniLiftJogDirection_Up:
            new = (positionNode)? positionNode->next: positionList.lastNode;
            break;
        case UniLiftJogDirection_Down:
            new = (positionNode)? positionNode->prev: positionList.firstNode;
            break;
    }
    if(new) {
        setPositionNode(new);
        return ((Position*) new->data)->id;
    }
    return -1;
}

bool UniLift_hasHomeSwitch() {
    return (setup.homeSwitch != NULL);
}

bool UniLift_getHomeSwitch() {
    ErrorMsgIf(!setup.homeSwitch, VEXOS_OPINVALID, "Lift does not have a home switch");
    return DigitalIn_get(setup.homeSwitch);
}

void UniLift_setPower(Power power) {
    setPositionNode(NULL);

    switch(setup.type) {
        case UniLiftType_Single:
            MotorGroup_setPower(setup.motors.single, power);
            break;
        case UniLiftType_Split:
            MotorGroup_setPower(setup.motors.split.left,  power);
            MotorGroup_setPower(setup.motors.split.right, power);
            break;
    }
}
