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

// Subsystem components //
DeclareSubsystem(UniDrive);

typedef enum {
    UniDriveType_Tank,
    UniDriveType_Holonomic,
    UniDriveType_HDrive
} UniDriveType;

#define UniDrive_PID_Default_kP     0.5
#define UniDrive_PID_Default_kI     0.0
#define UniDrive_PID_Default_kD     0.3

typedef struct {
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
            MotorGroup* leftRear;
            MotorGroup* rightRear;
            MotorGroup* leftFront;
            MotorGroup* rightFront;
        } holo;
        struct {
            MotorGroup* left;
            MotorGroup* right;
            MotorGroup* center;
        } H;
    } motors;
    struct {
        float kP, kI, kD;
    } pid;
} UniDriveSetup;

typedef enum {
    UniDriveAction_None,
    UniDriveAction_Move,
    UniDriveAction_Turn
} UniDriveAction;

// API methods to call //
UniDriveType UniDrive_getType();

void UniDrive_driveTank(Power left, Power right);
void UniDrive_driveHolo(Power leftFront, Power leftRear, Power rightFront, Power rightRear);
void UniDrive_driveH(Power left, Power right, Power center);

void UniDrive_autoBeginMove(float distance);
void UniDrive_autoBeginTurn(float angle);
bool UniDrive_autoIsComplete();
void UniDrive_autoEnd();
UniDriveAction UniDrive_autoGetAction();

// methods you must implement //
void UniDrive_configure(Subsystem*, UniDriveSetup* setup);
Command* UniDrive_getDefaultCommand(Subsystem*);

// Command Components //
DeclareCommandClass(UniDriveMove);
DeclareCommandClass(UniDriveTurn);
DeclareCommandClass(UniDriveWithJoystick);
void UniDriveWithJoystick_setPowerScaler(Command* cmd, PowerScaler*);

/********************************************************************
 * UniLift: Universal Lift                                          *
 ********************************************************************/

DeclareSubsystem(UniLift);

typedef enum {
    UniLiftType_Single,
    UniLiftType_Split,
} UniLiftType; 

#define UniLift_PID_Default_kP     0.5
#define UniLift_PID_Default_kI     0.0
#define UniLift_PID_Default_kD     0.3

typedef struct {
    UniLiftType type;
    float       gearRatio;
    union {
        MotorGroup* single;
        struct {
            MotorGroup* left;
            MotorGroup* right;
        } split;
    } motors;
    struct {
        float kP, kI, kD;
    } pid;
    DigitalIn* homeSwitch;
} UniLiftSetup;

typedef enum {
    UniLiftJogDirection_Up   = 1,
    UniLiftJogDirection_Down = -1
} UniLiftJogDirection;

// API methods to call //
void   UniLift_addPosition(int id, String name, float value);
String UniLift_getPositionName(int id);
float  UniLift_getPositionValue(int id);

int    UniLift_getPosition();
void   UniLift_setPosition(int id);
int    UniLift_jogPosition(UniLiftJogDirection dir);

bool   UniLift_hasHomeSwitch();
bool   UniLift_getHomeSwitch();
void   UniLift_resetPosition(int id);
void   UniLift_setPower(Power power);

// methods you must implement //
void UniLift_configure(Subsystem*, UniLiftSetup* setup);
Command* UniLift_getDefaultCommand(Subsystem*);

// Command Components //
DeclareCommandClass(UniLiftSet);
DeclareCommandClass(UniLiftJog);
DeclareCommandClass(UniLiftHome);

/********************************************************************
 * UniIntake: Universal Intake                                      *
 ********************************************************************/

DeclareSubsystem(UniIntake);

typedef struct {
    MotorGroup* motors;
} UniIntakeSetup;

// API methods to call //
void   UniIntake_addDirection(int id, String name, Power power);
String UniIntake_getDirectionName(int id);
Power  UniIntake_getDirectionPower(int id);

int    UniIntake_getDirection();
void   UniIntake_setDirection(int id);
void   UniIntake_setPower(Power power);

// methods you must implement //
void UniIntake_configure(Subsystem*, UniIntakeSetup* setup);
Command* UniIntake_getDefaultCommand(Subsystem*);

// Command Components //
DeclareCommandClass(UniIntakeSet);
