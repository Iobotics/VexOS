//
//  UniDriveWithJoystick.c
//  VexOS for Vex Cortex, Universal Robot Library
//
//  Created by Jeff Malins on 12/29/2012.
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

#include "CommandClass.h"
#include "UniBot.h"
#include "Error.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineCommandClass(UniDriveWithJoystick, {
    Joystick*    stick;
    PowerScaler* scaler;
});

static void constructor(va_list argp) {
    self->fields->stick = va_arg(argp, Joystick*);
    ErrorIf(self->fields->stick, VEXOS_ARGNULL);
    setArgs("%s", Joystick_getId(self->fields->stick));
    require(&UniDrive);
}

static void initialize() { }

static Power getPower(Power power) {
    return (self->fields->scaler)? 
                PowerScaler_get(self->fields->scaler, power): 
                power;
}

#define NUM_MOTORS  4

#define LEFT_FRONT  0
#define LEFT_REAR   1
#define RIGHT_FRONT 2
#define RIGHT_REAR  3

#define ABS(x)  (x < 0)? -(x): (x)

static void computeHolonomic(float x, float y, float rotation) {
    float speeds[NUM_MOTORS];

    // get raw speeds //
    speeds[LEFT_FRONT]  =  x + y + rotation;
    speeds[RIGHT_FRONT] = -x + y - rotation;
    speeds[LEFT_REAR]   = -x + y + rotation;
    speeds[RIGHT_REAR]  =  x + y - rotation;
    
    // normalize speeds //
    float max = ABS(speeds[0]);
    for(int i = 1; i < NUM_MOTORS; i++) {
        float temp = ABS(speeds[i]);
        if(max < temp) max = temp;
    }
    if(max > 1.0) {
        for(int i = 0; i< NUM_MOTORS; i++) {
            speeds[i] = speeds[i] / max;
        }
    }

    // set drive //
    UniDrive_driveHolo(speeds[LEFT_FRONT],  speeds[LEFT_REAR], 
                       speeds[RIGHT_FRONT], speeds[RIGHT_REAR]);

}

static void execute() { 
    switch(UniDrive_getType()) {
        case UniDriveType_Tank:
            UniDrive_driveTank(getPower(Joystick_getY(self->fields->stick, JoystickHand_Left)),
                               getPower(Joystick_getY(self->fields->stick, JoystickHand_Right)));
            break;
        case UniDriveType_Holonomic:
            computeHolonomic(getPower(Joystick_getX(self->fields->stick, JoystickHand_Left)),
                             getPower(Joystick_getY(self->fields->stick, JoystickHand_Right)),
                             getPower(Joystick_getX(self->fields->stick, JoystickHand_Right)));
            break;
        case UniDriveType_HDrive:
            UniDrive_driveH(getPower(Joystick_getY(self->fields->stick, JoystickHand_Left)),
                            getPower(Joystick_getY(self->fields->stick, JoystickHand_Right)),
                            getPower(Joystick_getX(self->fields->stick, JoystickHand_Right)));
            break;
    }
}

static bool isFinished() {
    return isTimedOut();
}

static void end() { }

static void interrupted() { }

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void UniDriveWithJoystick_setPowerScaler(Command* cmd, PowerScaler* scaler) {
    checkInstance(cmd);
    cmd->fields->scaler = scaler;
}
