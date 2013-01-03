//
//  PIDController.c
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
//  --------------------------------------------------------------------------
//
//  Some algorithms, structures and code organization used by this file have
//  been adapted from the WPILibJ software package. Those portions are:
//
//  Copyright (c) FIRST 2008-2012. All rights reserved.
//  
//  WPILibJ elements are distributed in this program in accordance to their 
//  original FIRST BSD License, available as 'BSD_License_for_WPILib_code.txt' 
//  present in this distribution.
//

#include "PID.h"
#include "Interrupt.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct PIDController {
    // configuration fields //
    PIDInput*  pidInput;
    PIDOutput* pidOutput;
    void*      state;
    bool       enabled;
    float      minIn, maxIn;
    float      tolerance;
    // PID algorithm structure //
    PIDState   data;
};

static void pidInterrupt(void* object) {
    PIDController* pid = object;

    if(pid->enabled) {
        // process the PID data //
        pid->data.input = pid->pidInput(pid->state);
        PID_calculate(&pid->data);
        pid->pidOutput(pid->state, pid->data.output);
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

PIDController* PIDController_new(PIDInput* input, PIDOutput* output, void* state) {
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);
    ErrorIf(input == NULL, VEXOS_ARGNULL);
    ErrorIf(output == NULL, VEXOS_ARGNULL);
    
    PIDController* pid = malloc(sizeof(PIDController));
    // set PID constants //
    pid->pidInput  = input;
    pid->pidOutput = output;
    pid->state     = state;
    // set defaults //
    pid->enabled   = false;
    pid->minIn     = 0.0;
    pid->maxIn     = 0.0;
    pid->tolerance = 0.0;
    // initialize with defaults //
    PID_initialize(&pid->data);
    
    // add the interrupt handler: at priority 9, it runs before     //
    // MotorGroup power updates, allowing low phase lag for sensors //
    Interrupt_add(pid, &pidInterrupt, 1, 9);
    return pid;
}

PIDController* PIDController_delete(PIDController* pid) {
    if(!pid) return NULL;
    if(pid->enabled) PIDController_setEnabled(pid, false);
    Interrupt_remove(pid, &pidInterrupt);
    free(pid);
    return pid;
}

void* PIDController_getState(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->state;
}

void PIDController_setPID(PIDController* pid, float kP, float kI, float kD) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorIf(kP < 0.0, VEXOS_ARGRANGE);
    ErrorIf(kI < 0.0, VEXOS_ARGRANGE);
    ErrorIf(kD < 0.0, VEXOS_ARGRANGE);
    ErrorIf(pid->enabled, VEXOS_OPINVALID);

    pid->data.kP = kP;
    pid->data.kI = kI;
    pid->data.kD = kD;
}

float PIDController_getP(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.kP;
}

float PIDController_getI(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.kI;
}

float PIDController_getD(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.kD;
}

void PIDController_getInputRange(PIDController* pid, float* min, float* max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);

    if(min) *min = pid->minIn;
    if(max) *max = pid->maxIn;
}

void PIDController_setInputRange(PIDController* pid, float min, float max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");
    ErrorIf(pid->enabled, VEXOS_OPINVALID);

    pid->minIn = min;
    pid->maxIn = max;
    PIDController_setSetpoint(pid, pid->data.command);
}

void PIDController_getOutputRange(PIDController* pid, float* min, float* max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);

    if(min) *min = pid->data.minOut;
    if(max) *max = pid->data.maxOut;
}

void PIDController_setOutputRange(PIDController* pid, float min, float max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");
    ErrorIf(pid->enabled, VEXOS_OPINVALID);

    pid->data.minOut = min;
    pid->data.maxOut = max;
}

bool PIDController_isEnabled(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->enabled;
}

void PIDController_setEnabled(PIDController* pid, bool value) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    if(!value) {
        pid->pidOutput(pid->state, 0.0);
    }
    pid->enabled = value;
}

void PIDController_setTolerance(PIDController* pid, float tolerance) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorIf(tolerance < 0, VEXOS_ARGRANGE);
    
    pid->tolerance = tolerance;
}

bool PIDController_onTarget(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return (ABS(pid->data.error) < pid->tolerance);
}

float PIDController_getError(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.error;
}

float PIDController_getOutput(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.output;
}

float PIDController_getSetpoint(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->data.command;
}

void PIDController_setSetpoint(PIDController* pid, float setpoint) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    if(pid->maxIn > pid->minIn) {
        if(setpoint < pid->minIn) {
            setpoint = pid->minIn;
        } else if(setpoint > pid->maxIn) {
            setpoint = pid->maxIn;
        }
    }

    pid->data.command = setpoint;
}
