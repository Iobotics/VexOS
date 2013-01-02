//
//  MotorGroup.h
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 01/01/2013.
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

#ifndef _MotorGroup_h
#define _MotorGroup_h

#include "Motor.h"
#include "PID.h"

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

typedef void (SpeedHandler)(MotorGroup* group);

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
    Power          powerDeadbandMin;
    Power          powerDeadbandMax;
    Power          powerSlewRate;
    float          outputScale;
    bool           feedbackEnabled;
    FeedbackType   feedbackType;
    Device*        feedbackDevice;
    float          feedbackScale;
    DigitalIn*     limitSwitchRev;
    DigitalIn*     limitSwitchFwd;
    volatile float position;
    volatile float lastPosition;
    volatile float speed;
    int            speedCycle;
    SpeedHandler*  speedHandler;
    bool           pidEnabled;
    PIDState       pid;
    float          pidTolerance;
    unsigned char  globaldataSlot;
};

SpeedHandler* MotorGroup_getSpeedHandler(MotorGroup* group);
void MotorGroup_setSpeedHandler(MotorGroup* group, SpeedHandler* handler);

#endif // _MotorGroup_h
