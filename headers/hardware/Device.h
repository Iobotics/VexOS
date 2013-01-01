//
//  Device.h
//  VexOS for Vex Cortex, Hardware Abstraction Layer
//
//  Created by Jeff Malins on 12/12/2012.
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

#ifndef _Device_h
#define _Device_h

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

struct Device {
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    Subsystem*    subsystem;
};

void Device_setSubsystem(Subsystem*);
void Device_addDigital(DigitalPort, DigitalPortMode, Device*);
void Device_addAnalog(AnalogPort, Device*);
void Device_addPWM(PWMPort, Device*);
void Device_addI2c(I2c, Device*);
void Device_setPWMExpander(PWMPort, PowerExpander*);
void Device_addUART(UARTPort, Device*);
void Device_addVirtualDevice(Device*);

#endif // _Device_h
