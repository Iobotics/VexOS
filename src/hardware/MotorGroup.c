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
#include "MotorGroup.h"
#include "Interrupt.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define MAX_MOTOR_POWER         127
#define SPEED_COMPUTE_CYCLES    5

// encoder ticks per revolution //
#define TicksPerRev_IME_393HT   627.2
#define TicksPerRev_IME_393HS   392.0
#define TicksPerRev_IME_269     240.448

// internal state fields //
static char nextGlobalDataSlot = GLOBALDATA_MOTORGROUP_STATE;

static bool    initialized;
static char    imeWatch;
static ImeData imeData[MAX_IME];

// called before group interrupts //
static void imeInterrupt(void* object) {
    if(!imeWatch) return;
    GetIntegratedMotorEncodersData(imeData);
}

// called for each group //
static void groupInterrupt(void* object) {
    MotorGroup* group = object;

    // make sure we process feedback //
    if(!group->feedbackEnabled) goto handle_power;

    // get the current position //
    Device* device = group->feedbackDevice;
    Motor*  motor  = NULL;
    GlobalDataValue gdata;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            // get the position //
            motor = (Motor*) device;
            gdata.floatValue = imeData[motor->i2c - 1].counter * group->feedbackScale;
            if(motor->reversed) {
                gdata.floatValue = -gdata.floatValue;
            }
            group->pid.input = gdata.floatValue;
            break;
        case FeedbackType_Encoder:
            gdata.floatValue = Encoder_get((Encoder*) group->feedbackDevice);
            GlobalData(group->globaldataSlot) = gdata.ulongValue;
            group->pid.input = gdata.floatValue * group->feedbackScale;
            break;
        case FeedbackType_Potentiometer:
            gdata.floatValue = AnalogIn_get((AnalogIn*) group->feedbackDevice);
            GlobalData(group->globaldataSlot) = gdata.ulongValue;
            group->pid.input = gdata.floatValue * group->feedbackScale;
            break;
        default: break;
    }
    // we computed with pid.input because it was non-volatile //
    // this is more efficient, we know volatile things won't  //
    // change in the ISR, but the compiler doesn't know that  //
    group->position = group->pid.input;
   
    // compute speed every 5 cycles (100ms), avoid glitch during startup // 
    group->speedCycle--;
    if(group->speedCycle <= 0) {
        // this is a valid countdown, compute based on feedback type //
        if(group->speedCycle == 0) {
            if(group->feedbackType == FeedbackType_IME) {
                // IME use the built-in period value to determine speed //
                int xspeed = imeData[motor->i2c - 1].speed;
                if((xspeed != 0) && (group->pid.input != group->lastPosition)) {
                    if(group->pid.input < group->lastPosition) xspeed = -xspeed;
                    // reversed engineered speed conversion formal //
                    group->speed = (125440.0 * ABS(group->feedbackScale) / xspeed);
                } else {
                    group->speed = 0.0;
                }
            } else {
                // other cases use delta in position over time //
                group->speed = (group->pid.input - group->lastPosition) 
                                / (SPEED_COMPUTE_CYCLES * INTERRUPT_PERIOD_SECONDS);
            }
            if(group->speedHandler) group->speedHandler(group);
        }
        group->lastPosition = group->pid.input;
        group->speedCycle   = (group->speedCycle == -1)? 1: SPEED_COMPUTE_CYCLES;
    }

    // run the PID loop, if PID is enabled //
    if(group->pidEnabled) {
        PID_calculate(&group->pid);
        group->powerRequested = group->pid.output;
    }

    // update the motors with open-loop functions //
handle_power:
    // handle limit switches //
    if(  (group->powerRequested < 0 && group->limitSwitchRev && DigitalIn_get(group->limitSwitchRev)) 
      || (group->powerRequested > 0 && group->limitSwitchFwd && DigitalIn_get(group->limitSwitchFwd))) 
    {
        group->powerActual = 0;
    } else {
        // check if there is something to do //
        if(group->powerActual == group->powerRequested) return;

        // handle slewing //
        if(group->powerRequested > group->powerActual) {
            group->powerActual += group->powerSlewRate;
            if(group->powerActual > group->powerRequested) {
                group->powerActual = group->powerRequested;
            }
        } else {
            group->powerActual -= group->powerSlewRate;
            if(group->powerActual < group->powerRequested) {
                group->powerActual = group->powerRequested;
            }
        }
    }

    // update the motors //
    ListNode* mnode = group->children.firstNode;
    while(mnode) {
        motor = mnode->data;
        float mpower = (motor->reversed)? -group->powerActual: group->powerActual; 
        SetMotor(motor->port, mpower * MAX_MOTOR_POWER); 
        mnode = mnode->next;
    }
}

static void initialize() {
    // register the IME watcher //
    Interrupt_add(NULL, &imeInterrupt, 1, 5);
    initialized = true;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

SpeedHandler* MotorGroup_getSpeedHandler(MotorGroup* group) {
    return group->speedHandler;
}

void MotorGroup_setSpeedHandler(MotorGroup* group, SpeedHandler* handler) {
    group->speedHandler = handler;
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
    ret->powerActual      = 0.0;
    ret->powerRequested   = 0.0;
    ret->powerDeadbandMin = 0.0;
    ret->powerDeadbandMax = 0.0;
    ret->powerSlewRate    = 2.0; // disabled: slew entire range in one cycle //
    ret->outputScale      = 1.0;
    ret->feedbackEnabled  = false;
    ret->feedbackType     = FeedbackType_None;
    ret->feedbackDevice   = NULL;
    ret->feedbackScale    = 1.0;
    ret->limitSwitchRev   = NULL;
    ret->limitSwitchFwd   = NULL;
    ret->position         = 0.0;
    ret->lastPosition     = 0.0;
    ret->speed            = 0.0;
    ret->speedCycle       = 0;  // forces a startup //
    ret->speedHandler     = NULL;
    ret->pidEnabled       = false;
    PID_initialize(&ret->pid);
    ret->pidTolerance     = (10.0 / 360); // motor within 10 degrees //
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
    group->feedbackType   = FeedbackType_IME;
    group->feedbackDevice = (Device*) motor;
    // set feedback ratio based on IME type //
    switch(type) {
        case MotorType_269:    group->feedbackScale = (1.0 / -TicksPerRev_IME_269);  break;
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

    // disable PID if manual power setting is used //
    if(group->pidEnabled) {
        MotorGroup_setPIDEnabled(group, false);
    }

    // clip to range //
    if(power < group->pid.minOut) {
        power = group->pid.minOut;
    } else if(power > group->pid.maxOut) {
        power = group->pid.maxOut;
    }

    // handle deadband //
    if(  (group->powerDeadbandMin < group->powerDeadbandMax)
      && (power > group->powerDeadbandMin)
      && (power < group->powerDeadbandMax))
    {
        power = 0;
    }

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

void MotorGroup_getDeadband(MotorGroup* group, Power* min, Power* max) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    if(min) *min = group->powerDeadbandMin;
    if(max) *max = group->powerDeadbandMax;
}

void MotorGroup_setDeadband(MotorGroup* group, Power min, Power max) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");

    group->powerDeadbandMin = min;
    group->powerDeadbandMax = max;
}

Power MotorGroup_getSlewTime(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return (group->powerSlewRate >= 2.0)? 0: group->powerSlewRate;
}

void MotorGroup_setSlewTime(MotorGroup* group, float time) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(time < 0, VEXOS_ARGRANGE);

    if(time > 0) {
        group->powerSlewRate = (INTERRUPT_PERIOD_SECONDS / time);
    } else {
        // for slew == 0, this will go full scale in one cycle //
        // which is the fastest we can do anyway with the ISR  //
        group->powerSlewRate = 2.0;
    }
}

DigitalIn* MotorGroup_getReverseLimitSwitch(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->limitSwitchRev;
}

void MotorGroup_setReverseLimitSwitch(MotorGroup* group, DigitalIn* input) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->limitSwitchRev = input;
}

bool MotorGroup_isReverseLimitOK(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return (group->limitSwitchRev)? !DigitalIn_get(group->limitSwitchRev): true;
}

DigitalIn* MotorGroup_getForwardLimitSwitch(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->limitSwitchFwd;
}

void MotorGroup_setForwardLimitSwitch(MotorGroup* group, DigitalIn* input) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->limitSwitchFwd = input;
}

bool MotorGroup_isForwardLimitOK(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return (group->limitSwitchFwd)? !DigitalIn_get(group->limitSwitchFwd): true;
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
    Encoder_preset(encoder, 0.0);
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
    AnalogIn_preset(pot, 0.0);
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
    if(!value) {
        if(group->pidEnabled) {
            MotorGroup_setPIDEnabled(group, false);
        }
        group->position     = 0.0;
        group->lastPosition = 0.0;
        group->speed        = 0.0;
    } else {
        group->speedCycle = 0;
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

    group->outputScale = scale;
}

float MotorGroup_getFeedbackScaleFactor(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->feedbackScale;
}

void MotorGroup_setFeedbackScaleFactor(MotorGroup* group, float scale) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(scale == 0.0, VEXOS_ARGINVALID);
    ErrorIf(group->feedbackType == FeedbackType_IME, VEXOS_OPINVALID);

    group->feedbackScale = scale;
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
    float sensorPos = (value / (group->outputScale * group->feedbackScale));
    float motorPos  = 0;
    
    Device* device = group->feedbackDevice;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            PresetIntegratedMotorEncoder(((Motor*) device)->port, (long) sensorPos);
            // there will be rounding errors because ticks are integers  //
            // change position to rounded values so we don't get phantom //
            // speed when the ISR sees a change of position from sensor  //
            motorPos = ((long) sensorPos) * group->feedbackScale;
            break;
        case FeedbackType_Encoder:
            Encoder_preset((Encoder*) device, sensorPos);
            // same as above //
            motorPos = Encoder_get((Encoder*) device) * group->feedbackScale;
            break;
        case FeedbackType_Potentiometer:
            AnalogIn_preset((AnalogIn*) device, sensorPos);
            // same as above //
            motorPos = AnalogIn_get((AnalogIn*) device) * group->feedbackScale;            
            break;
        default: 
            break;
    }
    // set last position to prevent a speed glitch     //
    // note that an interrupt between preset and this  //
    // statement can still glitch, but chances are low //
    group->position     = motorPos;
    group->lastPosition = motorPos;
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
    Motor* motor = NULL;
    long imePos  = 0;
    GlobalDataValue gdata;
    switch(group->feedbackType) {
        case FeedbackType_IME:
            motor = (Motor*) device;
            imePos = GetSavedCompetitionIme(motor->port);
            if(group->feedbackScale < 0) {
                imePos = -imePos;
            }
            PresetIntegratedMotorEncoder(motor->port, imePos);
            gdata.floatValue = imePos;
            break;
        case FeedbackType_Encoder:
            gdata.ulongValue = GlobalData(group->globaldataSlot);
            Encoder_preset((Encoder*) device, gdata.floatValue);
            break;
        case FeedbackType_Potentiometer:
            gdata.ulongValue = GlobalData(group->globaldataSlot);
            AnalogIn_preset((AnalogIn*) device, gdata.floatValue);
            break;
        default: 
            gdata.floatValue = 0;
            break;
    }
    group->position     = gdata.floatValue * group->feedbackScale;
    group->lastPosition = group->position;
    PrintToScreen("restore:  %f, %d\n", group->position, imePos);
    PrintToScreen("position: %f\n", group->position * group->outputScale);
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
    
    return group->pidTolerance * ABS(group->outputScale);
}

void MotorGroup_setTolerance(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    ErrorIf(value < 0, VEXOS_ARGINVALID);

    group->pidTolerance = (value / ABS(group->outputScale));
}

bool MotorGroup_onTarget(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);
    
    if(!group->pidEnabled) return true;
    return (ABS(group->pid.error) < group->pidTolerance);
}

float MotorGroup_getSetpoint(MotorGroup* group) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    return group->pid.command * group->outputScale;
}

void MotorGroup_setSetpoint(MotorGroup* group, float value) {
    ErrorIf(group == NULL, VEXOS_ARGNULL);

    group->pid.command = (value / group->outputScale);
}
