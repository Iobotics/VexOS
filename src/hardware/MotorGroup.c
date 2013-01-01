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
#include "Interrupt.h"
#include "PID.h"
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
    Subsystem*     subsystem;
    // device item fields //
    List           children;
    volatile Power powerActual;
    Power          powerRequested;
    Power          powerSlewRate;
    float          outputScale;
    bool           feedbackEnabled;
    FeedbackType   feedbackType;
    Device*        feedbackDevice;
    bool           feedbackReversed;
    float          feedbackScale;
    volatile float position;
    volatile float lastPosition;
    volatile float speed;
    bool           pidEnabled;
    PIDState       pid;
    float          pidTolerance;
    unsigned char  globaldataSlot;
};

// internal state fields //
static char nextGlobalDataSlot = GLOBALDATA_MOTORGROUP_STATE;

static bool    initialized;
static char    imeWatch;
static ImeData imeData[MAX_IME];

static void imeInterrupt(void* object) {
    if(!imeWatch) return;
    GetIntegratedMotorEncodersData(imeData);
}

static void groupInterrupt(void* object) {
    //MotorGroup* group = object;


}

static void initialize() {
    // register the IME watcher //
    Interrupt_add(NULL, &imeInterrupt, 1, 5);
    initialized = true;
}

// no attempt was made to optimize this ISR, test it as is, then optimize if //
// it starts to cause problems for the rest of the code.                     //
/*static void processISR() {
    // do not run during computations //
    if(VexOS_getRunMode() <= RunMode_Initialize) return;

    // general IME data //
    if(imeCapture && imeCount > 0) {
        GetIntegratedMotorEncodersData(ime);
    }

    ListNode* node  = processList.firstNode;
    ListNode* mnode = NULL;
    bool reverse;
    long sensor;
    while(node != NULL) {
        MotorGroup* group = node->data;
        switch(group->feedbackType) {
            case FeedbackType_IME:
                // read PID data from IME and write power to other motors //
                mnode = group->children.firstNode;
                if(!mnode) break;
                IME_GetPIDControlData(((Motor*) mnode->data)->port, 
                    &group->pidData.enabled, 
                    &group->pidData.setpoint, 
                    &group->pidData.power, 
                    &group->pidData.error,
                    &group->pidData.deltaError,
                    &group->pidData.sigmaError);
                reverse = ((Motor*) mnode->data)->reversed;
                mnode = mnode->next;
                while(mnode != NULL) {
                    if(!((Motor*) mnode->data)->reversed ^ reverse) {
                        SetMotor(((Motor*) mnode->data)->port, group->pidData.power);
                    } else {
                        SetMotor(((Motor*) mnode->data)->port, -group->pidData.power);
                    }
                    mnode = mnode->next;
                }
                group->power = group->pidData.power / MAX_MOTOR_POWER;
                break;
            case FeedbackType_Encoder:
            case FeedbackType_Potentiometer:
                // update the PID structure //
                if(!group->pidData.enabled) break;
                
                // get sensor value and save to globalData //
                if(group->feedbackType == FeedbackType_Encoder) {
                    sensor = Encoder_getRaw((Encoder*) group->feedbackDevice);
                } else {
                    sensor = AnalogIn_getRaw((AnalogIn*) group->feedbackDevice);
                }
                GlobalData(group->globaldataSlot) = sensor;

                // compute error //
                group->pidData.error = group->pidData.setpoint - sensor;
                
                // handle I //
                if(((group->pidData.sigmaError + group->pidData.error) * group->kI < MAX_MOTOR_POWER)
                   && ((group->pidData.sigmaError + group->pidData.error) * group->kI > -MAX_MOTOR_POWER)) {
                    group->pidData.sigmaError += group->pidData.error;
                }
    
                // handle P, D //
                group->pidData.power = (group->kP * group->pidData.error 
                                      + group->kI * group->pidData.sigmaError 
                                      + group->kD * (group->pidData.error - group->pidData.deltaError));
                group->pidData.deltaError = group->pidData.error;
    
                // limit output //
                if(group->pidData.power > MAX_MOTOR_POWER) {
                    group->pidData.power = MAX_MOTOR_POWER;
                } else if(group->pidData.power < -MAX_MOTOR_POWER) {
                    group->pidData.power = -MAX_MOTOR_POWER;
                }

                // set the motors // 
                mnode = group->children.firstNode;
                while(mnode != NULL) {
                    if(!((Motor*) mnode->data)->reversed) {
                        SetMotor(((Motor*) mnode->data)->port, group->pidData.power);
                    } else {
                        SetMotor(((Motor*) mnode->data)->port, -group->pidData.power);
                    }
                    mnode = mnode->next;
                }

                group->power = group->pidData.power / MAX_MOTOR_POWER;
                break;
            case FeedbackType_None:
                break;
        }
        node = node->next;
    }
}

#define startInterrupt()    RegisterImeInterruptServiceRoutine(processISR)
#define stopInterrupt()     UnRegisterImeInterruptServiceRoutine(processISR)

static void addProcessGroup(MotorGroup* group) {
    if(processList.nodeCount > 0 || imeCount > 0) {
        stopInterrupt();
    }
    List_insertLast(&processList, List_newNode(group));
    startInterrupt();
}

static void removeProcessGroup(MotorGroup* group) {
    stopInterrupt();
    ListNode* node = List_findNode(&processList, group);
    if(node) {
        List_remove(node);
    }
    if(processList.nodeCount > 0 || imeCount > 0) {
        startInterrupt();
    }
}*/

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

MotorGroup* MotorGroup_new(String name) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    MotorGroup* ret = malloc(sizeof(MotorGroup));
    ret->type             = DeviceType_MotorGroup;
    ret->name             = name;
    memset(&ret->children, 0, sizeof(List));
    ret->powerActual      = 0.0;
    ret->powerRequested   = 0.0;
    ret->powerSlewRate    = 2.0; // disabled: slew entire range in one cycle //
    ret->outputScale      = 1.0;
    ret->feedbackEnabled  = false;
    ret->feedbackType     = FeedbackType_None;
    ret->feedbackDevice   = NULL;
    ret->feedbackReversed = false;
    ret->feedbackScale    = 1.0;
    ret->speed            = 0.0;
    ret->pidEnabled       = false;
    PID_initialize(&ret->pid);
    ret->pidTolerance     = -1;
    ret->globaldataSlot   = 0;
    Device_addVirtualDevice((Device*) ret);

    // initialization //
    if(!initialized) initialize();
    
    // add a group-specific ISR //
    Interrupt_add(ret, &groupInterrupt, 1, 10);
    return ret;
}

void MotorGroup_add(MotorGroup* group, String name, PWMPort port, MotorType type, bool reversed) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Motor* motor = Motor_new(group, name, port, type, reversed, 0);
    List_insertLast(&group->children, List_newNode(motor));
}

void MotorGroup_addWithIME(MotorGroup* group, String name, PWMPort port, MotorType type, 
    bool reversed, I2c i2c) 
{
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    // make sure we don't already have an IME motor //
    ListNode* node = group->children.firstNode;
    ErrorMsgIf(node && ((Motor*) node->data)->i2c, VEXOS_OPINVALID, 
               "MotorGroup already has one IME: %s", group->name);

    // add motor at start of the list //
    Motor* motor = Motor_new(group, name, port, type, reversed, i2c);
    List_insertFirst(&group->children, List_newNode(motor));
    group->feedbackType     = FeedbackType_IME;
    group->feedbackDevice   = (Device*) motor;
    group->feedbackReversed = reversed;
    // set feedback ratio based on IME type //
    switch(type) {
        case MotorType_269:    group->feedbackScale = (1.0 / TicksPerRev_IME_269);   break;
        case MotorType_393_HT: group->feedbackScale = (1.0 / TicksPerRev_IME_393HT); break;
        case MotorType_393_HS: group->feedbackScale = (1.0 / TicksPerRev_IME_393HS); break;
        default: break;
    }
}

const List* MotorGroup_getMotorList(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return &group->children;
}

// open loop control //

Power MotorGroup_getPower(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->powerRequested;
}

Power MotorGroup_getActualPower(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->powerActual;
}

void MotorGroup_setPower(MotorGroup* group, Power power) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    if(group->powerRequested == power) return;

    // disable PID //
    if(group->pidEnabled) {
        MotorGroup_setPIDEnabled(group, false);
    }

    // power clipping //
    if(power > 1.0) power = 1.0;
    else if(power < -1.0) power = -1.0;

    // request the power, is set in the ISR //
    group->powerRequested = power;
}

void MotorGroup_getPowerRange(MotorGroup* group, Power* min, Power* max) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    if(min) *min = group->pid.minOut;
    if(max) *max = group->pid.maxOut;
}

void MotorGroup_setPowerRange(MotorGroup* group, Power min, Power max) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");

    group->pid.minOut = min;
    group->pid.maxOut = max;
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
    group->globaldataSlot = nextGlobalDataSlot++;
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
    group->globaldataSlot = nextGlobalDataSlot++;
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
    ErrorIf(VexOS_getRunMode() == RunMode_Setup, VEXOS_NOTINITIALIZED);

    if(group->feedbackEnabled == value) return;
    group->feedbackEnabled = value;
    if(!value && group->pidEnabled) {
        MotorGroup_setPIDEnabled(group, false);
    }
    switch(group->feedbackType) {
        case FeedbackType_IME:
            imeWatch += (value)? 1: -1;
            break;
        case FeedbackType_Encoder:
            Encoder_setEnabled((Encoder*) group->feedbackDevice, value);
            break;
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
    ErrorIf(group->feedbackType == FeedbackType_IME, VEXOS_OPINVALID);

    group->feedbackScale = scale;
}

bool MotorGroup_isFeedbackReversed(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackReversed;
}

void MotorGroup_setFeedbackReversed(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->feedbackReversed = value;
}

float MotorGroup_getPosition(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);
    
    if(!group->feedbackEnabled) return NAN;
    return group->position * group->outputScale;
}

void MotorGroup_presetPosition(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    if(!group->feedbackEnabled) return;
    float position = (value / (group->outputScale * group->feedbackScale));
    
    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            PresetIntegratedMotorEncoder(((Motor*) device)->port, (long) position);
            break;
        case FeedbackType_Encoder:
            Encoder_preset((Encoder*) device, position);
            break;
        case FeedbackType_Potentiometer:
            AnalogIn_preset((AnalogIn*) device, position);
            break;
        default: 
            break;
    }
    // set last position to prevent a speed glitch     //
    // note that an interrupt between preset and this  //
    // statement can still glitch, but chances are low //
    group->lastPosition = position;
}

float MotorGroup_getSpeed(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(VexOS_getRunMode() == RunMode_Setup, VEXOS_NOTINITIALIZED);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    if(!group->feedbackEnabled) return NAN;
    return group->speed * group->outputScale;
}

void MotorGroup_restorePosition(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(VexOS_getRunMode() == RunMode_Setup, VEXOS_NOTINITIALIZED);
    ErrorMsgIf(group->feedbackType == FeedbackType_None, VEXOS_OPINVALID,
               "MotorGroup has no feedback mechanism: %s", group->name);

    Device* device = group->feedbackDevice;
    PWMPort port;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            port = ((Motor*) device)->port;
            PresetIntegratedMotorEncoder(port, GetSavedCompetitionIme(port));
            break;
        case FeedbackType_Encoder:
            Encoder_presetRaw((Encoder*) device, GlobalData(group->globaldataSlot));
            break;
        case FeedbackType_Potentiometer:
            AnalogIn_presetRaw((AnalogIn*) device, GlobalData(group->globaldataSlot));
            break;
        default: 
            break;
    }
}

// closed loop control //

bool MotorGroup_isPIDEnabled(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pidEnabled;
}

void MotorGroup_setPIDEnabled(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(VexOS_getRunMode() == RunMode_Setup, VEXOS_NOTINITIALIZED);
    if(value == group->pidEnabled) return;

    // if enabling, enable feedback if not active already //
    if(value && !group->feedbackEnabled) {
        MotorGroup_setFeedbackEnabled(group, true);
    }

    group->pidEnabled = value;
    // if turning off, make sure motor power is zero //
    if(!value) {
        MotorGroup_setPower(group, 0.0);
    }
}

void MotorGroup_setPID(MotorGroup* group, float kP, float kI, float kD) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(kP < 0 || kI < 0 || kD < 0, VEXOS_ARGRANGE);

    group->pid.kP = kP;
    group->pid.kI = kI;
    group->pid.kD = kD;
}

float MotorGroup_getP(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.kP;
}

float MotorGroup_getI(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.kI;
}

float MotorGroup_getD(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.kD;
}

float MotorGroup_getError(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.error * group->outputScale;
}

float MotorGroup_getTolerance(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    
    return group->pidTolerance * group->outputScale;
}

void MotorGroup_setTolerance(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(value < 0, VEXOS_ARGINVALID);

    group->pidTolerance = (value / group->outputScale);
}

bool MotorGroup_onTarget(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    
    if(!group->pidEnabled) return true;
    return (group->pid.error > -group->pidTolerance)
        && (group->pid.error <  group->pidTolerance);
}

float MotorGroup_getSetpoint(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.command * group->outputScale;
}

void MotorGroup_setSetpoint(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->pid.command = (value / group->outputScale);
}
