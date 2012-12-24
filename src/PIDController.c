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

#include "API.h"

#include "VexOS.h"
#include "Error.h"

/********************************************************************
 * PIDController Structure                                          *
 ********************************************************************/

struct PIDController {
    // state object //
    void* state;
    // configuration fields //
    float p, i, d;
    float period;
    float minOutput, maxOutput;
    float minInput, maxInput;
    bool isContinuous;
    bool enabled;
    float tolerance;
    float setpoint;
    PIDInput* pidInput;
    PIDOutput* pidOutput;
    // state fields //
    float error, prevError, totalError;
    float result;
};

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

List pidControllers;

static void calculate(PIDController* pid) {
    if(!pid->enabled) return;
    
    pid->error = pid->setpoint - pid->pidInput(pid->state);
    if(pid->isContinuous) {
        if(Fabs(pid->error) > (pid->maxInput - pid->minInput) / 2.0) {
            if(pid->error > 0.0) {
                pid->error = pid->error - pid->maxInput + pid->minInput;
            } else {
                pid->error = pid->error + pid->maxInput - pid->minInput;
            }
        }
    }
    
    // handle I //
    if(((pid->totalError + pid->error) * pid->i < pid->maxOutput)
        && ((pid->totalError + pid->error) * pid->i > pid->minOutput)) {
        pid->totalError += pid->error;
    }
    
    // handle P, D //
    pid->result = (pid->p * pid->error + pid->i * pid->totalError + pid->d * (pid->error - pid->prevError));
    pid->prevError = pid->error;
    
    if(pid->result > pid->maxOutput) {
        pid->result = pid->maxOutput;
    } else if(pid->result < pid->minOutput) {
        pid->result = pid->minOutput;
    }
    pid->pidOutput(pid->state, pid->result);
}

static void pidISR() {
    ListNode* node = pidControllers.firstNode;
    while(node != NULL) {
        calculate(node->data);
        node = node->next;
    }
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

PIDController* PIDController_new(float p, float i, float d, PIDInput input, PIDOutput output, void* state) {
    ErrorIf(p < 0.0, VEXOS_ARGRANGE);
    ErrorIf(i < 0.0, VEXOS_ARGRANGE);
    ErrorIf(d < 0.0, VEXOS_ARGRANGE);
    ErrorIf(input == NULL, VEXOS_ARGNULL);
    ErrorIf(output == NULL, VEXOS_ARGNULL);
    
    PIDController* pid = malloc(sizeof(PIDController));
    // set PID constants //
    pid->p = p;
    pid->i = i;
    pid->d = d;
    pid->pidInput  = input;
    pid->pidOutput = output;
    pid->state     = state;
    // set defaults //
    pid->minInput     = 0.0;
    pid->maxInput     = 0.0;
    pid->minOutput    = 1.0;
    pid->maxOutput    = -1.0;
    pid->isContinuous = false;
    pid->enabled      = false;
    pid->setpoint     = 0.0;
    pid->tolerance    = 0.05;
    // clear PID state //
    pid->error        = 0.0;
    pid->prevError    = 0.0;
    pid->totalError   = 0.0;
    pid->result       = 0.0;
    
    // add to list //
    List_insertLast(&pidControllers, List_newNode(pid));
    if(pidControllers.nodeCount == 1) {
        RegisterImeInterruptServiceRoutine(&pidISR);
    }
    return pid;
}

PIDController* PIDController_delete(PIDController* pid) {
    if(!pid) return NULL;
    ListNode* node = List_findNode(&pidControllers, pid);
    if(node != NULL) {
        List_remove(node);
        if(pidControllers.nodeCount == 0) {
            UnRegisterImeInterruptServiceRoutine(&pidISR);
        }
    }
    return pid;
}

void* PIDController_getState(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->state;
}

float PIDController_get(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->result;
}

float PIDController_getError(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->error;
}

float PIDController_getP(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->p;
}

float PIDController_getI(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->i;
}

float PIDController_getD(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->d;
}

float PIDController_getSetpoint(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->setpoint;
}

bool PIDController_onTarget(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return (Fabs(pid->error) < pid->tolerance / 100 * (pid->maxInput - pid->minInput));
}

void PIDController_reset(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    PIDController_setEnabled(pid, false);
    pid->prevError  = 0.0;
    pid->totalError = 0.0;
    pid->result     = 0.0;
}

bool PIDController_isContinuous(PIDController* pid) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    return pid->isContinuous;
}

void PIDController_setContinuous(PIDController* pid, bool value) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    pid->isContinuous = value;
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

void PIDController_setInputRange(PIDController* pid, float min, float max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");

    pid->minInput = min;
    pid->maxInput = max;
    PIDController_setSetpoint(pid, pid->setpoint);
}

void PIDController_setOutputRange(PIDController* pid, float min, float max) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(min > max, VEXOS_ARGINVALID, "Lower bound is greater than upper bound");

    pid->minOutput = min;
    pid->maxOutput = max;
}

void PIDController_setPID(PIDController* pid, float p, float i, float d) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorIf(p < 0.0, VEXOS_ARGRANGE);
    ErrorIf(i < 0.0, VEXOS_ARGRANGE);
    ErrorIf(d < 0.0, VEXOS_ARGRANGE);
    
    pid->p = p;
    pid->i = i;
    pid->d = d;
}

void PIDController_setSetpoint(PIDController* pid, float setpoint) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    
    if(pid->maxInput > pid->minInput) {
        if (setpoint > pid->maxInput) {
            pid->setpoint = pid->maxInput;
        } else if (setpoint < pid->minInput) {
            pid->setpoint = pid->minInput;
        } else {
            pid->setpoint = setpoint;
        }
    } else {
        pid->setpoint = setpoint;
    }
}

void PIDController_setTolerance(PIDController* pid, float tolerance) {
    ErrorIf(pid == NULL, VEXOS_ARGNULL);
    ErrorIf(tolerance < 0 || tolerance > 1, VEXOS_ARGRANGE);
    
    pid->tolerance = tolerance;
}
