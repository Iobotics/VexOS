//
//  sys_Intake.c
//  VexOS
//
//  Created by Jeff Malins on 12/5/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Subsystem.h"
#include "robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(Intake);

static IntakeDirection currentDir = IntakeDirection_Stop;

static void constructor() { }

static void initDefaultCommand() {
    //setDefaultCommand(Command_new(&SetIntake, IntakeDirection_Stop));
}

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

static void setIntakePower(Power power) {
    SetMotor(MOTOR_INTAKE_L,  power);
    SetMotor(MOTOR_INTAKE_R, -power);
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

IntakeDirection Intake_getDirection() {
    return currentDir;
}

void Intake_setDirection(IntakeDirection dir) {
    if(currentDir == dir) return;
    switch(dir) {
        case IntakeDirection_Suck:
            setIntakePower(-1.0);
            break;
        case IntakeDirection_Blow:
            setIntakePower(1.0);
            break;
        case IntakeDirection_Stop:
        default:
            setIntakePower(0.0);
            break;
    }
    currentDir = dir;
}