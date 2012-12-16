//
//  Device.c
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

#include "Hardware.h"
#include "Device.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

#define DIGITAL_PORT_COUNT  12
#define ANALOG_PORT_COUNT   8
#define PWM_PORT_COUNT      10
#define UART_PORT_COUNT     2

typedef struct {
    Device*         device;
    DigitalPortMode mode;
} DigitalPortConfig;

typedef struct {
    Device*         device;
    PowerExpander*  expander;
} PWMPortConfig;

static unsigned char     lastDeviceId = 0;
static List              devices;
static DigitalPortConfig digitalPorts[DIGITAL_PORT_COUNT];
static Device*           analogPorts[ANALOG_PORT_COUNT];
static PWMPortConfig     pwmPorts[PWM_PORT_COUNT];
static Device*           uartPorts[UART_PORT_COUNT];

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Device_addDigital(DigitalPort port, DigitalPortMode mode, Device* device) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);
    ErrorMsgIf(digitalPorts[port].device, VEXOS_OPINVALID, 
               "Digital port is already allocated: %d", port);

    digitalPorts[port].device = device;
    digitalPorts[port].mode   = mode;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addAnalog(AnalogPort port, Device* device) {
    ErrorIf(port < AnalogPort_1 || port > AnalogPort_8, VEXOS_ARGRANGE);
    ErrorMsgIf(analogPorts[port], VEXOS_OPINVALID, 
               "Analog port is already allocated: %d", port);

    analogPorts[port] = device;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addPWM(PWMPort port, Device* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    ErrorMsgIf(pwmPorts[port].device, VEXOS_OPINVALID, 
               "PWM port is already allocated: %d", port);

    pwmPorts[port].device   = device;
    pwmPorts[port].expander = NULL;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_setPWMExpander(PWMPort port, PowerExpander* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    
    pwmPorts[port].expander = (PowerExpander*) device;
}

void Device_addUART(UARTPort port, Device* device) {
    ErrorIf(port < UARTPort_1 || port > UARTPort_2, VEXOS_ARGRANGE);
    ErrorMsgIf(uartPorts[port], VEXOS_OPINVALID, 
               "UART port is already allocated: %d", port);

    uartPorts[port] = device;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addVirtualDevice(Device* device) {
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_remove(Device* device) {
    ListNode* node = List_findNode(&devices, device);
    ErrorMsgIf(node == NULL, VEXOS_OPINVALID, "Device does not exist: %s", device->name);

    // remove ports by type //
    int i;
    switch(device->type) {
        // digital sensors //
        case DeviceType_BumpSwitch:
        case DeviceType_LimitSwitch:
        case DeviceType_Jumper:
        case DeviceType_QuadratureEncoder:
        case DeviceType_Encoder:
        case DeviceType_Sonar:
            for(i = 0; i < DIGITAL_PORT_COUNT; i++) {
                if(digitalPorts[i].device != device) continue;
                digitalPorts[i].device = NULL;
                digitalPorts[i].mode   = DigitalPortMode_Unassigned;
            }
            break;
        // analog sensors //
        case DeviceType_Potentiometer:
        case DeviceType_LineFollower: 
        case DeviceType_LightSensor:
        case DeviceType_Gyro:
        case DeviceType_Accelerometer:
            for(i = 0; i < ANALOG_PORT_COUNT; i++) {
                if(analogPorts[i] != device) continue;
                analogPorts[i] = NULL;
            }
            break;
        // digital outputs //
        case DeviceType_PneumaticValve:
        case DeviceType_LED:
            for(i = 0; i < DIGITAL_PORT_COUNT; i++) {
                if(digitalPorts[i].device != device) continue;
                digitalPorts[i].device = NULL;
                digitalPorts[i].mode   = DigitalPortMode_Unassigned;
            }
            break;
        // motors //
        case DeviceType_Motor:
        case DeviceType_Servo:
        case DeviceType_Speaker:
            for(i = 0; i < PWM_PORT_COUNT; i++) {
                if(pwmPorts[i].device != device) continue;
                pwmPorts[i].device = NULL;
            }
            break;
        // serial ports //
        case DeviceType_SerialPort:
            for(i = 0; i < UART_PORT_COUNT; i++) {
                if(uartPorts[i] != device) continue;
                uartPorts[i] = NULL;
            }
            break;
        // power expander //
        case DeviceType_PowerExpander:
            for(i = 0; i < ANALOG_PORT_COUNT; i++) {
                if(analogPorts[i] != device) continue;
                analogPorts[i] = NULL;
            }
            for(i = 0; i < PWM_PORT_COUNT; i++) {
                if(pwmPorts[i].expander != (PowerExpander*) device) continue;
                pwmPorts[i].expander = NULL;
            }
            break;
        // motor group //
        case DeviceType_MotorGroup:
            break;
    }
    List_remove(node);
    free(node);
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

String Device_getName(Device* device) {
    ErrorIf(device == NULL, VEXOS_ARGNULL);
    
    return device->name;
}

DeviceType Device_getType(Device* device) {
    ErrorIf(device == NULL, VEXOS_ARGNULL);
    
    return device->type;
}

Device* Device_getDigitalDevice(DigitalPort port) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);

    return digitalPorts[port].device;
}

DigitalPortMode Device_getDigitalPortMode(DigitalPort port) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);

    return digitalPorts[port].mode;
}

Device* Device_getAnalogDevice(AnalogPort port) {
    ErrorIf(port < AnalogPort_1 || port > AnalogPort_8, VEXOS_ARGRANGE);

    return analogPorts[port];
}

Device* Device_getPWMDevice(PWMPort port) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);

    return pwmPorts[port].device;
}

PowerExpander* Device_getPWMExpander(PWMPort port) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);

    return pwmPorts[port].expander;
}

Device* Device_getUARTDevice(UARTPort port) {
    ErrorIf(port < UARTPort_1 || port > UARTPort_2, VEXOS_ARGRANGE);

    return uartPorts[port];
}