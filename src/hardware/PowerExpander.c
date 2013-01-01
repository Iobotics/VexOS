//
//  PowerExpander.c
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

#include "API.h"

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

struct PowerExpander {
    // device header //
    unsigned char     deviceId;
    DeviceType        type;
    String            name;
    Subsystem*        subsystem;
    // device item fields //
    AnalogPort        statusPort;
    PowerExpanderType expandType;
    PWMPort           pwmPorts[4];
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

PowerExpander* PowerExpander_new(String name, PowerExpanderType type, AnalogPort port) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(type < PowerExpanderType_Rev_A1 || type > PowerExpanderType_Rev_A2,
               VEXOS_ARGINVALID, "Invalid PowerExpanderType: %d", type);

    PowerExpander* ret = malloc(sizeof(PowerExpander));
    ret->type       = DeviceType_PowerExpander;
    ret->name       = name;
    ret->expandType = type;
    ret->statusPort = port;
    memset(ret->pwmPorts, 0, 4 * sizeof(PWMPort));
    Device_addVirtualDevice((Device*) ret);
    if(port) {
        Device_addAnalog(port, (Device*) ret);
    }
    return ret;
}

PowerExpanderType PowerExpander_getType(PowerExpander* expand) {
    ErrorIf(expand == NULL, VEXOS_ARGNULL);
    
    return expand->expandType;
}

AnalogPort PowerExpander_getStatusPort(PowerExpander* expand) {
    ErrorIf(expand == NULL, VEXOS_ARGNULL);

    return expand->statusPort;
}

void PowerExpander_setPWMPorts(PowerExpander* expand, PWMPort port1, PWMPort port2, 
    PWMPort port3, PWMPort port4) 
{
    ErrorIf(expand == NULL, VEXOS_ARGNULL);
    // clear out old expander ports //
    if(expand->pwmPorts[0]) Device_setPWMExpander(expand->pwmPorts[0], NULL);
    if(expand->pwmPorts[1]) Device_setPWMExpander(expand->pwmPorts[1], NULL);
    if(expand->pwmPorts[2]) Device_setPWMExpander(expand->pwmPorts[2], NULL);
    if(expand->pwmPorts[3]) Device_setPWMExpander(expand->pwmPorts[3], NULL);
    // set the expander ports //
    expand->pwmPorts[0] = port1;
    Device_setPWMExpander(port1, expand);
    expand->pwmPorts[1] = port2;
    Device_setPWMExpander(port2, expand);
    expand->pwmPorts[2] = port3;
    Device_setPWMExpander(port3, expand);
    expand->pwmPorts[3] = port4;
    Device_setPWMExpander(port4, expand);
}

void PowerExpander_getPWMPorts(PowerExpander* expand, PWMPort* port1, PWMPort* port2, 
    PWMPort* port3, PWMPort* port4) 
{
    ErrorIf(expand == NULL, VEXOS_ARGNULL);
    *port1 = expand->pwmPorts[0];
    *port2 = expand->pwmPorts[1];
    *port3 = expand->pwmPorts[2];
    *port4 = expand->pwmPorts[3];
}

float PowerExpander_getBatteryVoltage(PowerExpander* expand) {
    ErrorIf(expand == NULL, VEXOS_ARGNULL);
    ErrorMsgIf(expand->statusPort == 0, VEXOS_OPINVALID, 
               "Expander has no status port set: %s", expand->name);

    float volts = ((GetAnalogInput(expand->statusPort) * 10.0f) / expand->expandType);
    return volts;
}
