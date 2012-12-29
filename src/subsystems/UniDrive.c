//
//  UniDrive.c
//  VexOS for Vex Cortex, Universal Robot Library
//
//  Created by Jeff Malins on 12/28/2012.
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

#include <math.h>

#include "Subsystem.h"
#include "UniBot.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineSubsystem(UniDrive);

static UniDriveSetup setup;
static bool pidAllowed = false;

static void constructor() {
    // set the default values //
    setup.system        = self;
    setup.type          = UniDriveType_Tank;
    setup.wheelDiameter = 4.0;
    setup.gearRatio     = 1.0;
    setup.driveWidth    = 7.5;

    // call the end-user constructor //
    UniDrive_configure(self, &setup);
}

static void initialize() {
    // check the geometry constants //
    ErrorIf(setup.wheelDiameter <= 0, VEXOS_ARGINVALID);
    ErrorIf(setup.driveWidth <= 0, VEXOS_ARGINVALID);

    // check the gearing //
    ErrorIf(setup.gearRatio <= 0, VEXOS_ARGINVALID);

    // validate we have the required motors //
    float outputScale = setup.gearRatio * setup.wheelDiameter * M_PI;
    switch(setup.type) {
        case UniDriveType_Tank:
            ErrorMsgIf(!setup.motors.tank.left || !setup.motors.tank.right, VEXOS_ARGNULL,
                       "Tank drive motors are not defined");
            MotorGroup_setOutputScaleFactor(setup.motors.tank.left,  outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.tank.right, outputScale);
            pidAllowed = (MotorGroup_getFeedbackType(setup.motors.tank.left)  != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.tank.right) != FeedbackType_None);
            break;
        case UniDriveType_Holonomic:
            ErrorMsgIf(!setup.motors.holo.leftFront  || !setup.motors.holo.leftRear
                    || !setup.motors.holo.rightFront || !setup.motors.holo.rightRear, 
                       VEXOS_ARGNULL, "Holonomic drive motors are not defined");
            MotorGroup_setOutputScaleFactor(setup.motors.holo.leftFront,  outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.holo.leftRear,   outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.holo.rightFront, outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.holo.rightRear,  outputScale);
            pidAllowed = (MotorGroup_getFeedbackType(setup.motors.holo.leftFront)  != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.holo.leftRear)   != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.holo.rightFront) != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.holo.rightRear)  != FeedbackType_None);
            break;
        case UniDriveType_HDrive:
            ErrorMsgIf(!setup.motors.H.left || !setup.motors.H.right || !setup.motors.H.center, 
                       VEXOS_ARGNULL, "H drive motors are not defined");
            MotorGroup_setOutputScaleFactor(setup.motors.H.left,   outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.H.right,  outputScale);
            MotorGroup_setOutputScaleFactor(setup.motors.H.center, outputScale);
            pidAllowed = (MotorGroup_getFeedbackType(setup.motors.H.left)   != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.H.right)  != FeedbackType_None)
                      && (MotorGroup_getFeedbackType(setup.motors.H.center) != FeedbackType_None);
            break;
    }

    setDefaultCommand(UniDrive_getDefaultCommand(self));
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void UniDrive_driveTank(Power left, Power right) {
    ErrorIf(setup.type != UniDriveType_Tank, VEXOS_OPINVALID);

    MotorGroup_setPower(setup.motors.tank.left,  left);
    MotorGroup_setPower(setup.motors.tank.right, right);
}

void UniDrive_driveHolo(Power leftFront, Power leftRear, Power rightFront, Power rightRear) {
    ErrorIf(setup.type != UniDriveType_Holonomic, VEXOS_OPINVALID);

    MotorGroup_setPower(setup.motors.holo.leftFront,  leftFront);
    MotorGroup_setPower(setup.motors.holo.leftRear,   leftRear);
    MotorGroup_setPower(setup.motors.holo.rightFront, rightFront);
    MotorGroup_setPower(setup.motors.holo.rightRear,  rightRear);
}

void UniDrive_driveH(Power left, Power right, Power center) {
    ErrorIf(setup.type != UniDriveType_HDrive, VEXOS_OPINVALID);

    MotorGroup_setPower(setup.motors.H.left,   left);
    MotorGroup_setPower(setup.motors.H.right,  right);
    MotorGroup_setPower(setup.motors.H.center, center);
}

void UniDrive_autoDrive(float distance) {
    ErrorMsgIf(!pidAllowed, VEXOS_OPINVALID, "UniDrive does not have full PID feedback");

}

void UniDrive_autoTurn(float angle) {
    ErrorMsgIf(!pidAllowed, VEXOS_OPINVALID, "UniDrive does not have full PID feedback");

}

