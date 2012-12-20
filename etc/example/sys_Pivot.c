//
//  sys_Pivot.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Pivot);

static PivotPosition currentPos = PivotPosition_Down;
static DigitalOut* valve;

static void constructor() { 
    valve = DigitalOut_newPneumaticValve("pivot valve", DIGITAL_PIVOT_VALVE);
}

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
            DigitalOut_set(valve, true);
            break;
        case PivotPosition_Up:
        default:
            DigitalOut_set(valve, false);
            break;
    }
    currentPos = pos;
}

PivotPosition Pivot_getPosition() {
    return currentPos;
}



