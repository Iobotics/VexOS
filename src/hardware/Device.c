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
#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

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
static Device*           i2cDevices[I2C_DEVICE_COUNT];
static Device*           uartPorts[UART_PORT_COUNT];

static void updateDigitalWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < DIGITAL_PORT_COUNT; i++) {
        DigitalPortConfig dpc = digitalPorts[i];
        PrintTextToGD(top + i, left, Color_Black, "%2d %s %.*s\n", i + 1, 
                (dpc.mode == DigitalPortMode_Input)?  "->":
                (dpc.mode == DigitalPortMode_Output)? "<-": "",
                width - 6, (dpc.device)? dpc.device->name: "");
    }
}

static void updateAnalogWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < ANALOG_PORT_COUNT; i++) {
        PrintTextToGD(top + i, left, Color_Black, "%2d %.*s\n", i + 1, 
                width - 3, (analogPorts[i])? analogPorts[i]->name: "");
    }
}

static void updatePWMWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < PWM_PORT_COUNT; i++) {
        PWMPortConfig ppc = pwmPorts[i];
        PrintTextToGD(top + i, left, Color_Black, "%2d %.*s\n", i + 1, 
                width - 3, (ppc.device)? ppc.device->name: "");
    }
}

static void updateUARTWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < UART_PORT_COUNT; i++) {
        PrintTextToGD(top + i, left, Color_Black, "%2d %.*s\n", i + 1, 
                width - 3, (uartPorts[i])? uartPorts[i]->name: "");
    }
}

static void updateI2CWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < 10; i++) {
        PrintTextToGD(top + i, left, Color_Black, "%2d %.*s\n", i + 1, 
                width - 3, "");
    }
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

void Device_addDigital(DigitalPort port, DigitalPortMode mode, Device* device) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);
    ErrorMsgIf(digitalPorts[port - 1].device, VEXOS_OPINVALID, 
               "Digital port is already allocated: %d", port);

    digitalPorts[port - 1].device = device;
    digitalPorts[port - 1].mode   = mode;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addAnalog(AnalogPort port, Device* device) {
    ErrorIf(port < AnalogPort_1 || port > AnalogPort_8, VEXOS_ARGRANGE);
    ErrorMsgIf(analogPorts[port - 1], VEXOS_OPINVALID, 
               "Analog port is already allocated: %d", port);

    analogPorts[port - 1] = device;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addPWM(PWMPort port, Device* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    ErrorMsgIf(pwmPorts[port - 1].device, VEXOS_OPINVALID, 
               "PWM port is already allocated: %d", port);

    pwmPorts[port - 1].device   = device;
    pwmPorts[port - 1].expander = NULL;
    if(List_indexOfData(&devices, device) == -1) {
        List_insertLast(&devices, List_newNode(device));
    }
    device->deviceId = ++lastDeviceId;
}

void Device_addI2c(I2c i2c, Device* device) {
    ErrorIf(i2c < I2c_1 || i2c > I2c_10, VEXOS_ARGRANGE);
    ErrorMsgIf(i2cDevices[i2c - 1], VEXOS_OPINVALID, 
               "I2C device is already allocated: %d", i2c);

    i2cDevices[i2c - 1] = device;
}

void Device_setPWMExpander(PWMPort port, PowerExpander* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    
    pwmPorts[port - 1].expander = (PowerExpander*) device;
}

void Device_addUART(UARTPort port, Device* device) {
    ErrorIf(port < UARTPort_1 || port > UARTPort_2, VEXOS_ARGRANGE);
    ErrorMsgIf(uartPorts[port - 1], VEXOS_OPINVALID, 
               "UART port is already allocated: %d", port);

    uartPorts[port - 1] = device;
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
    switch(device->type) {
        // digital sensors //
        case DeviceType_BumpSwitch:
        case DeviceType_LimitSwitch:
        case DeviceType_Jumper:
        case DeviceType_QuadratureEncoder:
        case DeviceType_Encoder:
        case DeviceType_Sonar:
            for(int i = 0; i < DIGITAL_PORT_COUNT; i++) {
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
            for(int i = 0; i < ANALOG_PORT_COUNT; i++) {
                if(analogPorts[i] != device) continue;
                analogPorts[i] = NULL;
            }
            break;
        // digital outputs //
        case DeviceType_PneumaticValve:
        case DeviceType_LED:
            for(int i = 0; i < DIGITAL_PORT_COUNT; i++) {
                if(digitalPorts[i].device != device) continue;
                digitalPorts[i].device = NULL;
                digitalPorts[i].mode   = DigitalPortMode_Unassigned;
            }
            break;
        // motors //
        case DeviceType_Motor:
            // check for I2C //
            for(int i = 0; i < I2C_DEVICE_COUNT; i++) {
                if(i2cDevices[i] != device) continue;
                i2cDevices[i] = NULL;
            }
        case DeviceType_Servo:
        case DeviceType_Speaker:
            for(int i = 0; i < PWM_PORT_COUNT; i++) {
                if(pwmPorts[i].device != device) continue;
                pwmPorts[i].device = NULL;
            }
            break;
        // serial devices //
        case DeviceType_SerialPort:
        case DeviceType_LCD:
            for(int i = 0; i < UART_PORT_COUNT; i++) {
                if(uartPorts[i] != device) continue;
                uartPorts[i] = NULL;
            }
            break;
        // power expander //
        case DeviceType_PowerExpander:
            for(int i = 0; i < ANALOG_PORT_COUNT; i++) {
                if(analogPorts[i] != device) continue;
                analogPorts[i] = NULL;
            }
            for(int i = 0; i < PWM_PORT_COUNT; i++) {
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

    return digitalPorts[port - 1].device;
}

DigitalPortMode Device_getDigitalPortMode(DigitalPort port) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);

    return digitalPorts[port - 1].mode;
}

Device* Device_getAnalogDevice(AnalogPort port) {
    ErrorIf(port < AnalogPort_1 || port > AnalogPort_8, VEXOS_ARGRANGE);

    return analogPorts[port - 1];
}

Device* Device_getPWMDevice(PWMPort port) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);

    return pwmPorts[port - 1].device;
}

PowerExpander* Device_getPWMExpander(PWMPort port) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);

    return pwmPorts[port - 1].expander;
}

Device* Device_getUARTDevice(UARTPort port) {
    ErrorIf(port < UARTPort_1 || port > UARTPort_2, VEXOS_ARGRANGE);

    return uartPorts[port - 1];
}

Device* Device_getByType(DeviceType type) {
    static ListNode* lastNode = NULL;

    ListNode* node = NULL;
    if(type) { 
        // first call //
        node = devices.firstNode;
    } else if(lastNode) {
        // subsequent call //
        node = lastNode->next;
    }

    while(node != NULL) {
        Device* device = node->data;
        if(device->type == type) {
            lastNode = node;
            return device;
        }
        node = node->next;
    }
    return NULL;
}

/********************************************************************
 * Public API (UI Hook)                                             *
 ********************************************************************/

Window* Device_getWindow(DeviceWindowType type) {
    static Window** windows = NULL;
    
    // initialize the cache if needed //
    if(!windows) {
        windows = malloc(DEVICE_WINDOW_COUNT * sizeof(Window*));
        memset(windows, 0, DEVICE_WINDOW_COUNT * sizeof(Window*));
    }
    // check for existing //
    if(windows[type]) return windows[type];

    // get windows by type //
    Window* win = NULL;
    switch(type) {
        case DeviceWindowType_Digital:
            win = Window_new("Digital Ports", &updateDigitalWindow);
            Window_setSize(win, 20, 12);
            break;
        case DeviceWindowType_Analog:
            win = Window_new("Analog Inputs", &updateAnalogWindow);
            Window_setSize(win, 20, 8);
            break;
        case DeviceWindowType_PWM:
            win = Window_new("PWM Outputs", &updatePWMWindow);
            Window_setSize(win, 20, 10);
            break;
        case DeviceWindowType_UART:
            win = Window_new("UART Ports", &updateUARTWindow);
            Window_setSize(win, 20, 2);
            break;
        case DeviceWindowType_I2C:
            win = Window_new("I2C Devices", &updateI2CWindow);
            Window_setSize(win, 20, 10);
            break;
    }
    return (windows[type] = win);
}

