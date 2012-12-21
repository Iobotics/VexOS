//
//  Motor.h
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/13/2012.
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

#ifndef _Motor_h
#define _Motor_h

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

struct Motor {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    PWMPort       port;
    MotorType     motorType;
    bool          reversed;
    I2c           i2c;
    MotorGroup*   parent;
    Power         power;
};

Motor* Motor_new(MotorGroup*, String, PWMPort, MotorType, bool, I2c);
Motor* Motor_delete(Motor*);
Power  Motor_getPower(Motor*);
void   Motor_setPower(Motor*, Power);

#endif // _Motor_h
