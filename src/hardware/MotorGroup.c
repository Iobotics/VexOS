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

typedef struct {
    unsigned char enabled;
    long          setpoint;
    int           power;
    long          error;
    long          deltaError;
    long          sigmaError;
} PIDData;

struct MotorGroup {
    // device header //
    unsigned char  deviceId;
    DeviceType     type;
    String         name;
    // device item fields //
    List           children;
    Power          power;
    bool           feedbackEnabled;
    FeedbackType   feedbackType;
    Device*        feedbackDevice;
    unsigned char  globaldataSlot;
    float          outputScale;
    float          feedbackScale;
    PIDData        pidData;
    bool           setpointReversed;
    float          kP, kI, kD;
    long           tolerance;
};

// internal state fields //
static char globaldataNextSlot = GLOBALDATA_MOTORGROUP_STATE;
static ImeData ime[MAX_IME];
static bool          imeCapture;
static unsigned char imeCount;
static List processList;

// no attempt was made to optimize this ISR, test it as is, then optimize if //
// it starts to cause problems for the rest of the code.                     //
static void processISR() {
    // do not run during computations //
    if(VexOS_getRunMode() <= RunMode_Initialize) return;

    // general IME data //
    if(imeCapture && imeCount > 0) {
        GetIntegratedMotorEncodersData(ime);
    }

    ListNode* node  = processList.firstNode;
    ListNode* mnode = NULL;
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
                mnode = mnode->next;
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
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

MotorGroup* MotorGroup_new(String name) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    MotorGroup* ret = malloc(sizeof(MotorGroup));
    ret->type             = DeviceType_MotorGroup;
    ret->name             = name;
    memset(&ret->children, 0, sizeof(List));
    ret->power            = 0.0;
    ret->feedbackEnabled  = false;
    ret->feedbackType     = FeedbackType_None;
    ret->feedbackDevice   = NULL;
    ret->globaldataSlot   = 0;
    ret->outputScale      = 1.0;
    ret->feedbackScale    = 1.0;
    memset(&ret->pidData, 0, sizeof(PIDData));
    ret->setpointReversed = false;
    ret->kP               = 0.0;
    ret->kI               = 0.0;
    ret->kD               = 0.0;
    ret->tolerance        = 32; 
    Device_addVirtualDevice((Device*) ret);
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
    group->feedbackDevice = (Device*) motor;
    group->feedbackType   = FeedbackType_IME;
    // set feedback ratio based on IME type //
    switch(type) {
        case MotorType_269:    group->feedbackScale = (360.0 / TicksPerRev_IME_269);   break;
        case MotorType_393_HT: group->feedbackScale = (360.0 / TicksPerRev_IME_393HT); break;
        case MotorType_393_HS: group->feedbackScale = (360.0 / TicksPerRev_IME_393HS); break;
        default: break;
    }
    group->setpointReversed = reversed;
}

const List* MotorGroup_getMotorList(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return &group->children;
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
    if(group->pidData.enabled) {
        MotorGroup_setPIDEnabled(group, false);
    }

    // power clipping //
    if(power > 1.0) power = 1.0;
    else if(power < -1.0) power = -1.0;

    // set the motors //
    group->power = power;
    ListNode* node = group->children.firstNode;
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
    group->globaldataSlot = globaldataNextSlot++;
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
    group->globaldataSlot = globaldataNextSlot++;
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
        case FeedbackType_IME:
            if(value) {
               if(++imeCount == 1) startInterrupt();
            } else {
                if(group->pidData.enabled) MotorGroup_setPIDEnabled(group, false);
                if(imeCount-- == 1) stopInterrupt();
            }
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
    group->pidData.setpoint = ticks;
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

void MotorGroup_restorePosition(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
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

    return group->pidData.enabled;
}

void MotorGroup_setPIDEnabled(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    if(value == group->pidData.enabled) return;

    // if enabling, enable feedback if not active already //
    if(value && !group->feedbackEnabled) {
        MotorGroup_setFeedbackEnabled(group, true);
    }

    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            if(value) {
                // if more than one motor, add to process groups //
                if(group->children.nodeCount > 1) {
                    addProcessGroup(group);
                }
                StartIntegratedMotorEncoderPID(((Motor*) device)->port, group->pidData.setpoint);
            } else {
                StopIntegratedMotorEncoderPID(((Motor*) device)->port);
                // if we have a slave list, remove it //
                if(group->children.nodeCount > 1) {
                    removeProcessGroup(group);
                }
            }
            group->pidData.enabled = value;
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            group->pidData.enabled = value;
            if(value) {
                addProcessGroup(group);
            } else {
                removeProcessGroup(group);
            }
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
            // constants are set above //
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

    return group->pidData.error * group->feedbackScale * group->outputScale;
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
            if(group->pidData.enabled) {
                MotorGroup_setPIDEnabled(group, false);
                // redefine the PID with new tolerance //
                DefineIntegratedMotorEncoderPID(((Motor*) device)->port, group->kP,
                    group->kI, group->kD, tolTicks);
                MotorGroup_setPIDEnabled(group, true);
            }
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            // tolerance is set above //
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
            return (-group->tolerance <= group->pidData.error 
                  && group->pidData.error <= group->tolerance);
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
    return true;
}

bool MotorGroup_isSetpointReversed(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->setpointReversed;
}

void MotorGroup_setSetpointReversed(MotorGroup* group, bool value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->setpointReversed = value;
}

float MotorGroup_getSetpoint(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pidData.setpoint * group->feedbackScale * group->outputScale;
}

void MotorGroup_setSetpoint(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    Device* device = group->feedbackDevice;
    long ticks = value / (group->outputScale * group->feedbackScale);
    if(group->setpointReversed) ticks = -ticks;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            UpdateSetpointIntegratedMotorEncoderPID(((Motor*) device)->port, ticks);
            break;
        case FeedbackType_Encoder:
        case FeedbackType_Potentiometer:
            group->pidData.setpoint = ticks;
            break;
        case FeedbackType_None:
            RaiseError(VEXOS_OPINVALID, "Feedback mechanism is required for PID control");
            break;
    }
}

