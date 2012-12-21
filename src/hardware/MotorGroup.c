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

#include "API.h"

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
    bool          feedbackEnabled;
    FeedbackType  feedbackType;
    Device*       feedbackDevice;
    PWMPort       imePort;
    double        outputRatio;
    double        feedbackRatio;
    bool          pidEnabled;
    double        kP, kI, kD;
    double        tolerance;
    double        setpoint;
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
            MotorGroup_remove(group, node->data);
            free(node);
            node = temp;
        }
        free(group->children);
        free(group);
    }
    return NULL;
}

void MotorGroup_add(MotorGroup* group, String name, PWMPort port, MotorType type, bool reversed) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Motor* motor = Motor_new(group, name, port, type, reversed, 0);
    List_insertLast(group->children, List_newNode(motor));
}

void MotorGroup_addWithIME(MotorGroup* group, String name, PWMPort port, MotorType type, 
    bool reversed, I2c i2c) 
{
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Motor* motor = Motor_new(group, name, port, type, reversed, i2c);
    List_insertLast(group->children, List_newNode(motor));
}

void MotorGroup_remove(MotorGroup* group, Motor* motor) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(motor == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(Motor_getGroup(motor) != group, VEXOS_OPINVALID, 
               "Motor is not part of MotorGroup: %s", motor->name);

    ListNode* node = List_findNode(group->children, motor);
    Motor_delete(motor);
    if(node) List_remove(node);
}

const List* MotorGroup_getMotorList(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->children;
}

// open loop control //

Power MotorGroup_getPower(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->power;
}

void MotorGroup_setPower(MotorGroup* group, Power power) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    // power clipping //
    if(power > 1.0) power = 1.0;
    else if(power < -1.0) power = -1.0;

    // set the motors //
    ListNode* node = group->children->firstNode;
    while(node != NULL) {
        Motor_setPower(node->data, power); 
        node = node->next;
    }
    group->power = power;
}

// feedback monitoring //

void MotorGroup_addEncoder(MotorGroup* group, Encoder* encoder) {
    ErrorIf(group == NULL,   VEXOS_ARGNULL);
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    Device* device = (Device*) encoder;
    ErrorIf(device->type != DeviceType_QuadratureEncoder, VEXOS_ARGINVALID);

}

void MotorGroup_addPotentiometer(MotorGroup* group, AnalogIn* pot) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(pot == NULL,   VEXOS_ARGNULL);
    Device* device = (Device*) pot;
    ErrorIf(device->type != DeviceType_Potentiometer, VEXOS_ARGINVALID);

}

Device* MotorGroup_getSensor(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackDevice;
}

FeedbackType MotorGroup_getFeedbackType(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackType;
}

bool MotorGroup_isFeedbackEnabled(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackEnabled;
}

void MotorGroup_setFeedbackEnabled(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

}

double MotorGroup_getOutputRatio(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->outputRatio;
}

void MotorGroup_setOutputRatio(MotorGroup* group, double ratio) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(ratio <= 0, VEXOS_ARGRANGE);

    group->outputRatio = ratio;
}

double MotorGroup_getFeedbackRatio(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackRatio;
}

void MotorGroup_setFeedbackRatio(MotorGroup* group, double ratio) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(ratio <= 0, VEXOS_ARGRANGE);

    group->feedbackRatio = ratio;
}

double MotorGroup_getPosition(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return 0.0; // FIXME //
}

void MotorGroup_presetPosition(MotorGroup* group, double value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

}

double MotorGroup_getSpeed(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return 0.0; // FIXME //
}

// closed loop control //

bool MotorGroup_isPIDEnabled(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pidEnabled;
}

void MotorGroup_setPIDEnabled(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
}

void MotorGroup_setPID(MotorGroup* group, double kP, double kI, double kD) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(kP < 0 || kI < 0 || kD < 0, VEXOS_ARGRANGE);

    group->kP = kP;
    group->kI = kI;
    group->kD = kD;
}

double MotorGroup_getP(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kP;
}

double MotorGroup_getI(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kI;
}

double MotorGroup_getD(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kD;
}

double MotorGroup_getError(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return 0.0; // FIXME //
}

double MotorGroup_getTolerance(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    
    return 0.0; // FIXME //
}

void MotorGroup_setTolerance(MotorGroup* group, double value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

}

bool MotorGroup_onTarget(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return false; // FIXME //
}

double MotorGroup_getSetpoint(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->setpoint;
}

void MotorGroup_setSetpoint(MotorGroup* group, double value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

}

