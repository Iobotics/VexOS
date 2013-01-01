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

#include "API.h"

#include "Hardware.h"
#include "Device.h"
#include "Motor.h"
#include "UserInterface.h"
#include "Error.h"

/********************************************************************
 * Private API                                                      *
 ********************************************************************/

// reversed engineered from easyC ctrl.dat file //
typedef enum {
    EasyCMotor_None      = 0,
    EasyCMotor_Standard  = 1,
    EasyCMotor_SmallIME  = 2,
    EasyCMotor_BigIME    = 3,
    EasyCMotor_BigIME_HS = 4
} EasyCMotor;

typedef struct {
    Device*         device;
    DigitalPortMode mode;
} DigitalPortConfig;

typedef struct {
    Device*         device;
    PowerExpander*  expander;
} PWMPortConfig;

static unsigned char     lastDeviceId = 0;
static Subsystem*        currentSubsystem;
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
        if(dpc.mode != DigitalPortMode_Unassigned) {
            PrintTextToGD(top + i, left, Color_Black, "%2d %2s %-5s %.*s\n", i + 1, 
                (dpc.mode == DigitalPortMode_Input)?  "->": "<-", 
                Device_getTypeName(dpc.device), 
                width - 11,
                dpc.device->name);
        } else {
            PrintTextToGD(top + i, left, Color_Grey, "%2d %.*s\n", i + 1, 
                width - 3, "(unassigned)");
        }
    }
}

static void updateAnalogWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < ANALOG_PORT_COUNT; i++) {
        if(analogPorts[i]) {
            PrintTextToGD(top + i, left, Color_Black, "%2d %-5s %.*s\n", i + 1, 
                Device_getTypeName(analogPorts[i]), 
                width - 8, 
                analogPorts[i]->name);
        } else {
            PrintTextToGD(top + i, left, Color_Grey, "%2d %.*s\n", i + 1, 
                width - 3, "(unassigned)");
        }
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
        if(ppc.device) {
            Motor* motor = (Motor*) ppc.device;
            I2c xi2c     = Motor_getI2c(motor);
            char* i2c = "";
            if(xi2c) {
                asprintf(&i2c, "%d", xi2c);
            }
            PrintTextToGD(top + i, left, Color_Black, "%2d %1s %-5s %2s %.*s\n", i + 1, 
                Motor_isReversed(motor)? "-": "+", 
                Device_getTypeName(ppc.device), i2c, 
                width - 14, 
                ppc.device->name);
            // clean up i2c string //
            if(strlen(i2c)) free(i2c);
        } else {
            PrintTextToGD(top + i, left, Color_Grey, "%2d   %.*s\n", i + 1, 
                width - 5, "(unassigned)");
        }
    }
}

static void updateUARTWindow(Window* win, bool full) {
    if(!full) return;
    Rect innerRect = Window_getInnerRect(win);
    unsigned char left  = innerRect.left;
    unsigned char top   = innerRect.top;
    unsigned char width = Window_getWidth(win);

    for(int i = 0; i < UART_PORT_COUNT; i++) {
        if(uartPorts[i]) {
            PrintTextToGD(top + i, left, Color_Black, "%2d %-5s %.*s\n", i + 1, 
                Device_getTypeName(uartPorts[i]), 
                width - 8, 
                uartPorts[i]->name);
        } else {
            PrintTextToGD(top + i, left, Color_Grey, "%2d %.*s\n", i + 1, 
                width - 3, "(unassigned)");
        }
    }
}

static bool addDevice(Device* device) {
    if(List_indexOfData(&devices, device) != -1) return false;
    List_insertLast(&devices, List_newNode(device));

    device->deviceId  = ++lastDeviceId;
    device->subsystem = currentSubsystem;
    return true;
}

/********************************************************************
 * Protected API                                                    *
 ********************************************************************/

// hidden method, not in header //
void Device_configureCortex() {
    // set the Cortex port directions //
    unsigned char dirs[DIGITAL_PORT_COUNT];
    for(int i = 0; i < DIGITAL_PORT_COUNT; i++) {
        switch(digitalPorts[i].mode) {
            case DigitalPortMode_Output: 
                dirs[i] = 0; 
                break;
            case DigitalPortMode_Input:
            case DigitalPortMode_Unassigned:
                dirs[i] = 1;
                break;
        }
    }
    DefineControllerIO(dirs[0], dirs[1], dirs[2], dirs[3], dirs[4],  dirs[5],
                       dirs[6], dirs[7], dirs[8], dirs[9], dirs[10], dirs[11]);

    // set the I2c ID values for each motor port //
    bool initIMEs = false;
    unsigned char motors[PWM_PORT_COUNT];
    for(int i = 0; i < PWM_PORT_COUNT; i++) {
        motors[i] = i + 1;
    }
    for(int i = 0; i < PWM_PORT_COUNT; i++) {
        // make sure we have a motor //
        Device* device = pwmPorts[i].device;
        if(!device || device->type != DeviceType_Motor) continue;
        // see if motor has an I2c //
        I2c i2c = Motor_getI2c((Motor*) device);
        if(!i2c) continue;
        // if so, swap with the motor that current holds that ID //
        for(int j = 0; j < PWM_PORT_COUNT; j++) {
            if(motors[j] != i2c) continue;
            motors[j] = motors[i];
            break;
        }
        motors[i] = i2c;
        initIMEs  = true;
    }
    DefineImeTable(motors[0], motors[1], motors[2], motors[3], motors[4],
                   motors[5], motors[6], motors[7], motors[8], motors[9]);
    
    // set the motor types using easyC values //
    unsigned char mtypes[PWM_PORT_COUNT];
    for(int i = 0; i < PWM_PORT_COUNT; i++) {
        Device* device = pwmPorts[i].device;
        if(!device) {
            mtypes[i] = EasyCMotor_None;
            continue;
        }
        switch(device->type) {
            case DeviceType_Motor:
                if(Motor_getI2c((Motor*) device)) {
                    switch(Motor_getMotorType((Motor*) device)) {
                        case MotorType_269:    mtypes[i] = EasyCMotor_SmallIME;  break;
                        case MotorType_393_HT: mtypes[i] = EasyCMotor_BigIME;    break;
                        case MotorType_393_HS: mtypes[i] = EasyCMotor_BigIME_HS; break;
                        default: mtypes[i] = EasyCMotor_Standard; break;
                    }
                } else {
                    mtypes[i] = EasyCMotor_Standard;
                }
                break;
            case DeviceType_Servo:
            default:
                mtypes[i] = EasyCMotor_None;
                break;
        }
    }
    DefineMotorTypes(mtypes[0], mtypes[1], mtypes[2], mtypes[3], mtypes[4], 
                     mtypes[5], mtypes[6], mtypes[7], mtypes[8], mtypes[9]);

    // if we are using IMEs, initialize them //
    if(initIMEs) {
        SetSaveCompetitionIme(true); // save state //
        InitIntegratedMotorEncoders();
    }
}

void Device_setSubsystem(Subsystem* sys) {
    currentSubsystem = sys;
}

void Device_addDigital(DigitalPort port, DigitalPortMode mode, Device* device) {
    ErrorIf(port < DigitalPort_1 || port > DigitalPort_12, VEXOS_ARGRANGE);
    ErrorMsgIf(digitalPorts[port - 1].device, VEXOS_OPINVALID, 
               "Digital port is already allocated: %d", port);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    digitalPorts[port - 1].device = device;
    digitalPorts[port - 1].mode   = mode;
    addDevice(device);
}

void Device_addAnalog(AnalogPort port, Device* device) {
    ErrorIf(port < AnalogPort_1 || port > AnalogPort_8, VEXOS_ARGRANGE);
    ErrorMsgIf(analogPorts[port - 1], VEXOS_OPINVALID, 
               "Analog port is already allocated: %d", port);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    analogPorts[port - 1] = device;
    addDevice(device);
}

void Device_addPWM(PWMPort port, Device* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    ErrorMsgIf(pwmPorts[port - 1].device, VEXOS_OPINVALID, 
               "PWM port is already allocated: %d", port);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    pwmPorts[port - 1].device   = device;
    pwmPorts[port - 1].expander = NULL;
    addDevice(device);
}

void Device_addI2c(I2c i2c, Device* device) {
    ErrorIf(i2c < I2c_1 || i2c > I2c_10, VEXOS_ARGRANGE);
    ErrorMsgIf(i2cDevices[i2c - 1], VEXOS_OPINVALID, 
               "I2C device is already allocated: %d", i2c);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    i2cDevices[i2c - 1] = device;
    addDevice(device);
}

void Device_setPWMExpander(PWMPort port, PowerExpander* device) {
    ErrorIf(port < PWMPort_1 || port > PWMPort_10, VEXOS_ARGRANGE);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    pwmPorts[port - 1].expander = device;
    addDevice((Device*) device);
}

void Device_addUART(UARTPort port, Device* device) {
    ErrorIf(port < UARTPort_1 || port > UARTPort_2, VEXOS_ARGRANGE);
    ErrorMsgIf(uartPorts[port - 1], VEXOS_OPINVALID, 
               "UART port is already allocated: %d", port);
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);

    uartPorts[port - 1] = device;
    addDevice(device);
}

void Device_addVirtualDevice(Device* device) {
    ErrorIf(VexOS_getRunMode() != RunMode_Setup, VEXOS_HARDWARELOCK);
    addDevice(device);
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

String Device_getTypeName(Device* device) {
    ErrorIf(device == NULL, VEXOS_ARGNULL);

    switch(device->type) {
        case DeviceType_BumpSwitch:         return "BUMP";
        case DeviceType_LimitSwitch:        return "LIMIT";
        case DeviceType_Jumper:             return "JUMP";
        case DeviceType_QuadratureEncoder:  return "QDENC";
        case DeviceType_Encoder:            return "ENC";
        case DeviceType_Sonar:              return "SONAR";
        // analog sensors //
        case DeviceType_Potentiometer:      return "POT";
        case DeviceType_LineFollower:       return "LINE";
        case DeviceType_LightSensor:        return "LIGHT";
        case DeviceType_Gyro:               return "GYRO";
        case DeviceType_Accelerometer:      return "ACCEL";
        // digital outputs //
        case DeviceType_PneumaticValve:     return "PNEUM";
        case DeviceType_LED:                return "LED";
        // miscellaneous devices //
        case DeviceType_PowerExpander:      return "EXPAN";
        case DeviceType_LCD:                return "LCD";
        case DeviceType_SerialPort:         return "SERIAL";
        case DeviceType_Speaker:            return "SPEAK";
        case DeviceType_MotorGroup:         return "MGRP";
        // motors //
        case DeviceType_Servo:              return "SERVO";
        case DeviceType_Motor:
            switch(Motor_getMotorType((Motor*) device)) {
                case MotorType_269:    return "269";
                case MotorType_393_HT: return "393HT";
                case MotorType_393_HS: return "393HS";
                default: return "MOTOR";
            }
            break;
        default: return "DEVICE";
    }
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

Device* Device_getI2cDevice(I2c i2c) {
    ErrorIf(i2c < I2c_1 || i2c > I2c_10, VEXOS_ARGRANGE);

    return i2cDevices[i2c - 1];
}

const List* Device_getDeviceList() {
    return &devices;
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
            Window_setSize(win, 28, 12);
            break;
        case DeviceWindowType_Analog:
            win = Window_new("Analog Inputs", &updateAnalogWindow);
            Window_setSize(win, 28, 8);
            break;
        case DeviceWindowType_PWM:
            win = Window_new("PWM Outputs", &updatePWMWindow);
            Window_setSize(win, 40, 10);
            break;
        case DeviceWindowType_UART:
            win = Window_new("UART Ports", &updateUARTWindow);
            Window_setSize(win, 28, 2);
            break;
    }
    return (windows[type] = win);
}

