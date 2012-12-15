//
//  sys_Pivot.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Pivot);

static PivotPosition currentPos = PivotPosition_Down;

static void constructor() { }

static void initDefaultCommand() {
    //setDefaultCommand(Command_new(&PivotSet, PivotPosition_Down));
}

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

void Pivot_setPosition(PivotPosition pos) {
    if(currentPos == pos) return;
    switch(pos) {
        case PivotPosition_Down:
            SetDigitalOutput(DIGITAL_PIVOT_VALVE, true);
            break;
        case PivotPosition_Up:
        default:
            SetDigitalOutput(DIGITAL_PIVOT_VALVE, false);
            break;
    }
    currentPos = pos;
}

PivotPosition Pivot_getPosition() {
    return currentPos;
}
