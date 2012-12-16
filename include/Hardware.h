//
//  Hardware.h
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

#ifndef _Hardware_h
#define _Hardware_h

#include "VexOS.h"

/********************************************************************
 * Master Device Enumerations                                       *
 ********************************************************************/

typedef enum { 
    // digital sensors //
    DeviceType_BumpSwitch = 1, 
    DeviceType_LimitSwitch,
    DeviceType_Jumper,
    DeviceType_QuadratureEncoder, 
    DeviceType_Encoder,
    DeviceType_Sonar, 
    // analog sensors //
    DeviceType_Potentiometer, 
    DeviceType_LineFollower, 
    DeviceType_LightSensor, 
    DeviceType_Gyro, 
    DeviceType_Accelerometer,
    // digital outputs //
    DeviceType_PneumaticValve,
    DeviceType_LED,
    // miscellaneous devices //,
    DeviceType_PowerExpander,
    DeviceType_LCD,
    DeviceType_SerialPort,
    DeviceType_MotorGroup,
    // motors //
    DeviceType_Motor,
    DeviceType_Servo,
    DeviceType_Speaker
} DeviceType;

typedef enum {
    DigitalPortMode_Unassigned,
    DigitalPortMode_Output,
    DigitalPortMode_Input
} DigitalPortMode;

/********************************************************************
 * VEX Cortex Hardware                                              *
 ********************************************************************/

typedef enum DigitalPort {
    DigitalPort_1 = 1, DigitalPort_2, DigitalPort_3, DigitalPort_4, DigitalPort_5,
    DigitalPort_6,     DigitalPort_7, DigitalPort_8, DigitalPort_9, DigitalPort_10,
    DigitalPort_11,    DigitalPort_12
} DigitalPort;

typedef enum AnalogPort {
    AnalogPort_1 = 1, AnalogPort_2, AnalogPort_3, AnalogPort_4, AnalogPort_5,
    AnalogPort_6,     AnalogPort_7, AnalogPort_8
} AnalogPort;

typedef enum PWMPort {
    PWMPort_1 = 1, PWMPort_2, PWMPort_3, PWMPort_4, PWMPort_5,
    PWMPort_6,     PWMPort_7, PWMPort_8, PWMPort_9, PWMPort_10
} PWMPort;

typedef enum UARTPort {
    UARTPort_1 = 1, UARTPort_2
} UARTPort;

/********************************************************************
 * Public Object Definitions                                        *
 ********************************************************************/

// base device class //
typedef struct Device        Device;
// subclasses //
typedef struct DigitalIn     DigitalIn;
typedef struct Encoder       Encoder;
typedef struct Sonar         Sonar;
typedef struct AnalogIn      AnalogIn;
typedef struct Gyro          Gyro;
typedef struct Accelerometer Accelerometer;
typedef struct DigitalOut    DigitalOut;
typedef struct Speaker       Speaker;
typedef struct PowerExpander PowerExpander;
typedef struct LCD           LCD;
typedef struct SerialPort    SerialPort;
typedef struct Motor         Motor;
typedef struct MotorGroup    MotorGroup;
typedef struct Servo         Servo;

/********************************************************************
 * Public API: Device (applies to all Device types with cast)       *
 ********************************************************************/

String          Device_getName(Device*);
DeviceType      Device_getType(Device*);
Device*         Device_getDigitalDevice(DigitalPort);
DigitalPortMode Device_getDigitalPortMode(DigitalPort);
Device*         Device_getAnalogDevice(AnalogPort);
Device*         Device_getPWMDevice(PWMPort);
PowerExpander*  Device_getPWMExpander(PWMPort);
Device*         Device_getUARTDevice(UARTPort);
Device*         Device_getByType(DeviceType);

/********************************************************************
 * Public API: DigitalIn                                            *
 ********************************************************************/

typedef enum {
    InterruptMode_Disabled,
    InterruptMode_RisingEdge,
    InterruptMode_FallingEdge
} InterruptMode;

DigitalIn*    DigitalIn_newBump(String, DigitalPort);
DigitalIn*    DigitalIn_newLimit(String, DigitalPort);
DigitalIn*    DigitalIn_newJumper(String, DigitalPort);
DigitalIn*    DigitalIn_delete(DigitalIn*);
DigitalPort   DigitalIn_getPort(DigitalIn*);
bool          DigitalIn_get(DigitalIn*);
InterruptMode DigitalIn_getInterruptMode(DigitalIn*);
void          DigitalIn_setInterruptMode(DigitalIn*, InterruptMode);
bool          DigitalIn_getInterrupted(DigitalIn*);
Button*       DigitalIn_getButton(DigitalIn*);

/********************************************************************
 * Public API: Encoder                                              *
 ********************************************************************/

#define TicksPerRev_QUAD_ENCODER    360.0
#define TicksPerRev_OLD_ENCODER     100.0

Encoder*    Encoder_newQuadrature(String, DigitalPort, DigitalPort, bool);
Encoder*    Encoder_new(String, DigitalPort);
Encoder*    Encoder_delete(Encoder*);
DigitalPort Encoder_getPort(Encoder*);
DigitalPort Encoder_getPort2(Encoder*);
bool        Encoder_isEnabled(Encoder*);
void        Encoder_setEnabled(Encoder*, bool);
long        Encoder_get(Encoder*);
void        Encoder_set(Encoder*, long);

/********************************************************************
 * Public API: Sonar                                                *
 ********************************************************************/

Sonar* Sonar_new(String, DigitalPort, DigitalPort);
Sonar* Sonar_delete(Sonar*);
DigitalPort Sonar_getInputPort(Sonar*);
DigitalPort Sonar_getOutputPort(Sonar*);
bool   Sonar_isEnabled(Sonar*);
void   Sonar_setEnabled(Sonar*, bool);
int    Sonar_getDistanceInches(Sonar*);
float  Sonar_getDistanceCentimeters(Sonar*);

/********************************************************************
 * Public API: AnalogIn                                             *
 ********************************************************************/

AnalogIn*  AnalogIn_newPotentiometer(String, AnalogPort);
AnalogIn*  AnalogIn_newLineFollower(String, AnalogPort);
AnalogIn*  AnalogIn_newLightSensor(String, AnalogPort);
AnalogIn*  AnalogIn_delete(AnalogIn*);
AnalogPort AnalogIn_getPort(AnalogIn*);
int        AnalogIn_get(AnalogIn*);

/********************************************************************
 * Public API: Gyro                                                 *
 ********************************************************************/

#define GYRO_DEFAULT_DEADBAND 3

Gyro*      Gyro_new(String, AnalogPort);
Gyro*      Gyro_delete(Gyro*);
AnalogPort Gyro_getPort(Gyro*);
void       Gyro_init(Gyro*);
bool       Gyro_isEnabled(Gyro*);
void       Gyro_setEnabled(Gyro*, bool);
int        Gyro_getDeadband(Gyro*);
void       Gyro_setDeadband(Gyro*, int);
float      Gyro_getAngleDegrees(Gyro *);

/********************************************************************
 * Public API: Accelerometer                                        *
 ********************************************************************/

Accelerometer* Accelerometer_new(String, AnalogPort);
Accelerometer* Accelerometer_delete(Accelerometer*);
AnalogPort     Accelerometer_getPort(Accelerometer*);
void           Accelerometer_init(Accelerometer*);
bool           Accelerometer_isEnabled(Accelerometer*);
void           Accelerometer_setEnabled(Accelerometer*, bool);
float          Accelerometer_getAccelerationG(Accelerometer*);

/********************************************************************
 * Public API: DigitalOut                                           *
 ********************************************************************/

DigitalOut* DigitalOut_newPneumaticValve(String, DigitalPort);
DigitalOut* DigitalOut_newLED(String, DigitalPort);
DigitalOut* DigitalOut_delete(DigitalOut*);
DigitalPort DigitalOut_getPort(DigitalOut*);
bool        DigitalOut_get(DigitalOut*);
void        DigitalOut_set(DigitalOut*, bool);

/********************************************************************
 * Public API: Motor                                                *
 ********************************************************************/

typedef enum {
    MotorType_None,
    MotorType_269    = 269,
    MotorType_393_HT = 393,
    MotorType_393_HS = 394
} MotorType;

typedef enum {
    I2c_1 = 1, I2c_2, I2c_3, I2c_4, I2c_5,
    I2c_6,     I2c_7, I2c_8, I2c_9, I2c_10
} I2c;

// encoder ticks per revolution //
#define TicksPerRev_IME_393HT       627.2
#define TicksPerRev_IME_393HS       392.0
#define TicksPerRev_IME_293         240.448

Motor*      Motor_new(String, PWMPort, MotorType);
Motor*      Motor_newWithIME(String, PWMPort, MotorType, I2c);
Motor*      Motor_delete(Motor*);
PWMPort     Motor_getPort(Motor*);
MotorType   Motor_getMotorType(Motor*);
I2c         Motor_getI2c(Motor*);
MotorGroup* Motor_getGroup(Motor*);
Power       Motor_get(Motor*);
void        Motor_set(Motor*, Power);

/********************************************************************
 * Public API: MotorGroup                                           *
 ********************************************************************/

MotorGroup* MotorGroup_new(String);
MotorGroup* MotorGroup_delete(MotorGroup*);
void        MotorGroup_addMotor(MotorGroup*, Motor*);
void        MotorGroup_removeMotor(MotorGroup*, Motor*);
const List* MotorGroup_getMotorList(MotorGroup*);
Power       MotorGroup_get(MotorGroup*);
void        MotorGroup_set(MotorGroup*, Power);

/********************************************************************
 * Public API: Servo                                                *
 ********************************************************************/

#define SERVO_TRAVEL_DEGREES 100

Servo*  Servo_new(String, PWMPort);
Servo*  Servo_delete(Servo*);
PWMPort Servo_getPort(Servo*);
float   Servo_getAngleDegrees(Servo*);
void    Servo_setAngleDegrees(Servo*, float);

/********************************************************************
 * Public API: PowerExpander                                        *
 ********************************************************************/

typedef enum {
    PowerExpanderType_Rev_A1 = 456,
    PowerExpanderType_Rev_A2 = 700
} PowerExpanderType;

PowerExpander*    PowerExpander_new(String, PowerExpanderType, AnalogPort);
PowerExpander*    PowerExpander_delete(PowerExpander*);
PowerExpanderType PowerExpander_getType(PowerExpander*);
AnalogPort        PowerExpander_getStatusPort(PowerExpander*);
void              PowerExpander_setPWMPorts(PowerExpander*, PWMPort, PWMPort, PWMPort, PWMPort);
void              PowerExpander_getPWMPorts(PowerExpander*, PWMPort*, PWMPort*, PWMPort*, PWMPort*);
float             PowerExpander_getBatteryVoltage(PowerExpander*);

/********************************************************************
 * Public API: LCD                                                  *
 ********************************************************************/

// note: there are more LCD methods in UserInterface.h //
LCD*     LCD_new(String, UARTPort);
LCD*     LCD_delete(LCD*);
UARTPort LCD_getPort(LCD*);
void     LCD_setBacklight(LCD*, bool);
bool     LCD_getBacklight(LCD*);

/********************************************************************
 * Public API: SerialPort                                           *
 ********************************************************************/

typedef enum {
    BaudRate_300   = 300,   BaudRate_600   = 600,   BaudRate_1200   = 1200,  
    BaudRate_2400  = 2400,  BaudRate_4800  = 4800,  BaudRate_9600   = 9600,    
    BaudRate_14400 = 14400, BaudRate_19200 = 19200, BaudRate_28800  = 28800, 
    BaudRate_38400 = 38400, BaudRate_57600 = 57600, BaudRate_115200 = 115200
} BaudRate;

typedef enum {
    SerialDataBits_8 = 0x0000,
    SerialDataBits_9 = 0x1000
} SerialDataBits;

typedef enum {
    SerialStopBits_1   = 0x0000,
    SerialStopBits_0_5 = 0x1000,
    SerialStopBits_2   = 0x2000,
    SerialStopBits_1_5 = 0x3000
} SerialStopBits;

typedef enum {
    SerialParity_None = 0x0000,
    SerialParity_Even = 0x0400,
    SerialParity_Odd  = 0x0600
} SerialParity;

// bit maskable //
typedef enum {
    SerialFlowControl_None = 0x0000,
    SerialFlowControl_RTS  = 0x0100,
    SerialFlowControl_CTS  = 0x0200
} SerialFlowControl;

// bit maskable //
typedef enum {
    SerialMode_RX = 0x0004,
    SerialMode_TX = 0x0008
} SerialMode;

typedef struct {
    SerialDataBits    dataBits;
    SerialStopBits    stopBits;
    SerialParity      parity;
    SerialFlowControl flowControl;
    SerialMode        mode;
} SerialOptions;

SerialPort*   SerialPort_new(String, UARTPort);
void          SerialPort_open(SerialPort*, BaudRate);
void          SerialPort_openWithOptions(SerialPort*, BaudRate, SerialOptions);
BaudRate      SerialPort_getBaudRate(SerialPort*);
SerialOptions SerialPort_getOptions(SerialPort*);
unsigned char SerialPort_getByteCount(SerialPort*);
unsigned char SerialPort_readByte(SerialPort*);
void          SerialPort_writeByte(SerialPort*, unsigned char byte);

#endif // _Hardware_h

