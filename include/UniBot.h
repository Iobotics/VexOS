//
//  UniBot.h
//  VexOS for Vex Cortex, Universal Robot Library
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

#include "VexOS.h"
#include "Hardware.h"

/********************************************************************
 * UniDrive: Universal Drivetrain                                   *
 ********************************************************************/

DeclareSubsystem(UniDrive);

typedef enum {
    UniDriveType_Tank,
    UniDriveType_Holonomic,
    UniDriveType_HDrive
} UniDriveType;

typedef struct {
    Subsystem*   system;
    UniDriveType type;
    float        wheelDiameter;
    float        driveWidth;
    float        gearRatio;
    union {
        struct {
            MotorGroup* left;
            MotorGroup* right;
        } tank;
        struct {
            MotorGroup* leftFront;
            MotorGroup* leftRear;
            MotorGroup* rightFront;
            MotorGroup* rightRear;
        } holo;
        struct {
            MotorGroup* left;
            MotorGroup* right;
            MotorGroup* center;
        } H;
    } motors;
} UniDriveSetup;

void UniDrive_driveTank(Power left, Power right);
void UniDrive_driveHolo(Power leftFront, Power leftRear, Power rightFront, Power rightRear);
void UniDrive_driveH(Power left, Power right, Power center);

void UniDrive_autoDrive(float distance);
void UniDrive_autoTurn(float angle);

// methods you must implement //
void UniDrive_configure(Subsystem*, UniDriveSetup* setup);
Command* UniDrive_getDefaultCommand(Subsystem*);
