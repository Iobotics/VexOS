//
//  PID.h
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

#ifndef _PID_h
#define _PID_h

#include "VexOS.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

typedef struct {
    // input and output //
    float command;
    float input;
    float output;
    // configuration fields //
    float kP, kI, kD;
    float minIn,  maxIn;
    float minOut, maxOut;
    bool  isContinuous;
    // state fields //
    float error, deltaError, sigmaError;
} PIDState;

void PID_initialize(PIDState* pid);
void PID_calculate(PIDState* pid);

#endif // _PID_h
