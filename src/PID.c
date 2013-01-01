//
//  PID.c
//  VexOS for Vex Cortex
//
//  Created by Jeff Malins on 12/31/2012.
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

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void PID_initialize(PIDState* pid) {
    pid->kP           = 0.0;
    pid->kI           = 0.0;
    pid->kD           = 0.0;
    pid->minIn        = -1.0;
    pid->maxIn        = 1.0;
    pid->minOut       = -1.0;
    pid->maxOut       = 1.0;
    pid->isContinuous = false;
    pid->error        = 0.0;
    pid->deltaError   = 0.0;
    pid->sigmaError   = 0.0;
}

void PID_calculate(PIDState* pid) {
    // limit input //
    float input = pid->input;
    if(input > pid->maxIn) {
        input = pid->maxIn;
    } else if(input < pid->minIn) {
        input = pid->minIn;
    }

    // compute error //
    float error = pid->command - input;
    // handle input that wraps around (i.e. absolute encoder) //
    if(pid->isContinuous) {
        if(ABS(pid->error) > (pid->maxIn - pid->minIn) / 2.0) {
            if(error > 0.0) {
                error += pid->minIn - pid->maxIn;
            } else {
                error += pid->maxIn - pid->minIn;
            }
        }
    }
    
    // accumulate error if not at limits, prevents "wind-up" //
    float x_iterm = (pid->sigmaError + error) * pid->kI;
    if((x_iterm < pid->maxOut) && (x_iterm > pid->minOut)) {
        pid->sigmaError += error;
    }
    // differentiate error //
    pid->deltaError = error - pid->error;
    pid->error      = error;
    
    // compute the result //
    float result = (pid->kP * error)
                 + (pid->kI * pid->sigmaError)
                 + (pid->kD * pid->deltaError);
    
    if(result > pid->maxOut) {
        result = pid->maxOut;
    } else if(result < pid->minOut) {
        result = pid->minOut;
    }
    pid->output = result;
}

