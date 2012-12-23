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

#define MAX_MOTOR_POWER     127

// encoder ticks per revolution //
#define TicksPerRev_IME_393HT       627.2
#define TicksPerRev_IME_393HS       392.0
#define TicksPerRev_IME_269         240.448

struct MotorGroup {
    // device header //
    unsigned char  deviceId;
    DeviceType     type;
    String         name;
    // device item fields //
    List*          children;
    Power          power;
    bool           feedbackEnabled;
    FeedbackType   feedbackType;
    Device*        feedbackDevice;
    float          outputScale;
    float          feedbackScale;
    PIDController* controller; // used when IME is not present //
    bool           pidEnabled;
    float          kP, kI, kD;
    long           tolerance;
    long           setpoint;
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
    ret->type            = DeviceType_MotorGroup;
    ret->name            = name;
    ret->children        = List_new();
    ret->power           = 0.0;
    ret->feedbackEnabled = false;
    ret->feedbackType    = FeedbackType_None;
    ret->feedbackDevice  = NULL;
    ret->outputScale     = 1.0;
    ret->feedbackScale   = 1.0;
    ret->controller      = NULL;
    ret->pidEnabled      = false;
    ret->kP              = 0.0;
    ret->kI              = 0.0;
    ret->kD              = 0.0;
    ret->tolerance       = 32; 
    ret->setpoint        = 0;
    Device_addVirtualDevice((Device*) ret);
    return ret;
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

    // make sure we don't already have an IME motor //
    ListNode* node = group->children->firstNode;
    ErrorMsgIf(node && ((Motor*) node->data)->i2c, VEXOS_OPINVALID, 
               "MotorGroup already has one IME: %s", group->name);

    // add motor at start of the list //
    Motor* motor = Motor_new(group, name, port, type, reversed, i2c);
    List_insertFirst(group->children, List_newNode(motor));
    group->feedbackDevice = (Device*) motor;
    group->feedbackType   = FeedbackType_IME;
    // set feedback ratio based on IME type //
    switch(type) {
        case MotorType_269:    group->feedbackScale = (360.0 / TicksPerRev_IME_269);   break;
        case MotorType_393_HT: group->feedbackScale = (360.0 / TicksPerRev_IME_393HT); break;
        case MotorType_393_HS: group->feedbackScale = (360.0 / TicksPerRev_IME_393HS); break;
        default: break;
    }
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
    if(group->power == power) return;

    // disable PID //
    if(group->pidEnabled) {
        MotorGroup_setPIDEnabled(group, false);
    }

    // power clipping //
    if(power > 1.0) power = 1.0;
    else if(power < -1.0) power = -1.0;

    // set the motors //
    group->power = power;
    ListNode* node = group->children->firstNode;
    while(node != NULL) {
        Motor* motor = node->data;
        Power mpower = power;
        if(motor->reversed) mpower *= -1.0;
        SetMotor(motor->port, (int) (mpower * MAX_MOTOR_POWER));
        node = node->next;
    }
}

// feedback monitoring //

void MotorGroup_addEncoder(MotorGroup* group, Encoder* encoder) {
    ErrorIf(group == NULL,   VEXOS_ARGNULL);
    ErrorIf(encoder == NULL, VEXOS_ARGNULL);
    Device* device = (Device*) encoder;
    ErrorIf(device->type != DeviceType_QuadratureEncoder, VEXOS_ARGINVALID);
    ErrorMsgIf(group->feedbackType != FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup already has a feedback mechanism: %s", group->name);

    group->feedbackType   = FeedbackType_Encoder;
    group->feedbackDevice = device;
    group->feedbackScale  = Encoder_getScaleFactor(encoder);
}

void MotorGroup_addPotentiometer(MotorGroup* group, AnalogIn* pot) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(pot == NULL,   VEXOS_ARGNULL);
    Device* device = (Device*) pot;
    ErrorIf(device->type != DeviceType_Potentiometer, VEXOS_ARGINVALID);
    ErrorMsgIf(group->feedbackType != FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup already has a feedback mechanism: %s", group->name);

    group->feedbackType   = FeedbackType_Potentiometer;
    group->feedbackDevice = device;
    group->feedbackScale  = AnalogIn_getScaleFactor(pot);
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
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    if(group->feedbackEnabled == value) return;
    group->feedbackEnabled = value;
    switch(group->feedbackType) {
        case FeedbackType_Encoder:
            Encoder_setEnabled((Encoder*) group->feedbackDevice, value);
            break;
        case FeedbackType_IME:
        case FeedbackType_Potentiometer:
        default: 
            break;
    }
}

float MotorGroup_getOutputScaleFactor(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->outputScale;
}

void MotorGroup_setOutputScaleFactor(MotorGroup* group, float scale) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(scale <= 0, VEXOS_ARGRANGE);

    group->outputScale = scale;
}

float MotorGroup_getFeedbackScaleFactor(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackScale;
}

void MotorGroup_setFeedbackScaleFactor(MotorGroup* group, float scale) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(scale <= 0, VEXOS_ARGRANGE);

    group->feedbackScale = scale;
}

float MotorGroup_getPosition(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);
    
    if(!group->feedbackEnabled) return 0.0;

    Device* device = group->feedbackDevice;
    float scale = group->outputScale * group->feedbackScale;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            return GetIntegratedMotorEncoder(((Motor*) device)->port) * scale;
        case FeedbackType_Encoder:
            return Encoder_getRaw((Encoder*) device) * scale;
        case FeedbackType_Potentiometer:
            return AnalogIn_getRaw((AnalogIn*) device) * scale;
        default: 
            return 0.0;
    }
}

void MotorGroup_presetPosition(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    if(!group->feedbackEnabled) return;

    Device* device = group->feedbackDevice;
    long ticks = value / (group->outputScale * group->feedbackScale);
    group->setpoint = ticks;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            PresetIntegratedMotorEncoder(((Motor*) device)->port, ticks);
            break;
        case FeedbackType_Encoder:
            Encoder_presetRaw((Encoder*) device, ticks);
            break;
        case FeedbackType_Potentiometer:
            AnalogIn_presetRaw((AnalogIn*) device, ticks);
            break;
        default: 
            break;
    }
}

float MotorGroup_getSpeed(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    if(!group->feedbackEnabled) return 0.0;

    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            // returns rev/sec -> convert to degrees/sec @ output //
            return GetIntegratedMotorEncoderSpeed(((Motor*) device)->port) 
                * 360.0 * group->outputScale;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
        default: 
            return 0.0;
    }
}

// closed loop control //

bool MotorGroup_isPIDEnabled(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pidEnabled;
}

void MotorGroup_setPIDEnabled(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    // if enabling, enable feedback if not active already //
    if(value && !group->feedbackEnabled) {
        MotorGroup_setFeedbackEnabled(group, true);
    }

    Device* device = group->feedbackDevice;
    group->pidEnabled = value;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            if(value) {
                StartIntegratedMotorEncoderPID(((Motor*) device)->port, group->setpoint);
            } else {
                StopIntegratedMotorEncoderPID(((Motor*) device)->port);
            }
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
}

void MotorGroup_setPID(MotorGroup* group, float kP, float kI, float kD) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(kP < 0 || kI < 0 || kD < 0, VEXOS_ARGRANGE);

    group->kP = kP;
    group->kI = kI;
    group->kD = kD;

    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            // define PID //
            DefineIntegratedMotorEncoderPID(((Motor*) device)->port, kP, kI, kD, 
                    group->tolerance);
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
}

float MotorGroup_getP(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kP;
}

float MotorGroup_getI(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kI;
}

float MotorGroup_getD(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->kD;
}

float MotorGroup_getError(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return MotorGroup_getPosition(group) - MotorGroup_getSetpoint(group);
}

float MotorGroup_getTolerance(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    
    return group->tolerance * group->feedbackScale * group->outputScale;
}

void MotorGroup_setTolerance(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(value < 0, VEXOS_ARGINVALID);

    Device* device = group->feedbackDevice;
    long tolTicks = value / (group->outputScale * group->feedbackScale);
    group->tolerance = tolTicks;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            if(group->pidEnabled) {
                // redefine the PID with new tolerance //
                DefineIntegratedMotorEncoderPID(((Motor*) device)->port, group->kP,
                    group->kI, group->kD, tolTicks);
            }
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
            break;
        case FeedbackType_None:
            break;
    }
}

bool MotorGroup_onTarget(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            return OnTargetIntegratedMotorEncoderPID(((Motor*) device)->port);
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
    return true;
}

float MotorGroup_getSetpoint(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->setpoint * group->feedbackScale * group->outputScale;
}

void MotorGroup_setSetpoint(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Device* device = group->feedbackDevice;
    long ticks = value / (group->outputScale * group->feedbackScale);
    group->setpoint = ticks;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            UpdateSetpointIntegratedMotorEncoderPID(((Motor*) device)->port, ticks);
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            RaiseError(VEXOS_OPINVALID, "Not supported with current feedback mechanism");
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
}

