//
//  Sonar.c
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

struct Sonar {
    // device header //
    unsigned char deviceId;
    DeviceType    type;
    String        name;
    // device item fields //
    DigitalPort   inputPort;
    DigitalPort   outputPort;
    bool          enabled;
};

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

Sonar* Sonar_new(String name, DigitalPort inputPort, DigitalPort outputPort) {
    ErrorIf(name == NULL, VEXOS_ARGNULL);

    Sonar* ret = malloc(sizeof(Sonar));
    ret->type       = DeviceType_Sonar;
    ret->name       = name;
    ret->inputPort  = inputPort;
    ret->outputPort = outputPort;
    ret->enabled    = false;
    Device_addDigital(inputPort,  DigitalPortMode_Input,  (Device*) ret);
    Device_addDigital(outputPort, DigitalPortMode_Output, (Device*) ret);
    return ret;
}

DigitalPort Sonar_getInputPort(Sonar* sonar) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);
    
    return sonar->inputPort;
}

DigitalPort Sonar_getOutputPort(Sonar* sonar) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);
    
    return sonar->outputPort;
}

bool Sonar_isEnabled(Sonar* sonar) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);

    return sonar->enabled;
}

void Sonar_setEnabled(Sonar* sonar, bool value) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);

    if(value) {
        StartUltrasonic(sonar->inputPort, sonar->outputPort);
    } else {
        StopUltrasonic(sonar->inputPort, sonar->outputPort);
    }
    sonar->enabled = value;
}

int Sonar_getDistanceInches(Sonar* sonar) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);

    return GetUltrasonic(sonar->inputPort, sonar->outputPort);
}

float Sonar_getDistanceCentimeters(Sonar* sonar) {
    ErrorIf(sonar == NULL, VEXOS_ARGNULL);

    return Sonar_getDistanceInches(sonar) * 2.54;
}

