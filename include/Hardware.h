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
    // serial devices //
    DeviceType_LCD,
    DeviceType_SerialPort,
    // miscellaneous devices //,
    DeviceType_PowerExpander,
    DeviceType_MotorGroup,
    DeviceType_Speaker,
    // motors //
    DeviceType_Motor,
    DeviceType_Servo
} DeviceType;

typedef enum {
    DigitalPortMode_Unassigned,
    DigitalPortMode_Output,
    DigitalPortMode_Input
} DigitalPortMode;

/********************************************************************
 * VEX Cortex Hardware                                              *
 ********************************************************************/

#define DIGITAL_PORT_COUNT  12
typedef enum DigitalPort {
    DigitalPort_1 = 1, DigitalPort_2, DigitalPort_3, DigitalPort_4, DigitalPort_5,
    DigitalPort_6,     DigitalPort_7, DigitalPort_8, DigitalPort_9, DigitalPort_10,
    DigitalPort_11,    DigitalPort_12
} DigitalPort;

#define ANALOG_PORT_COUNT   8
typedef enum AnalogPort {
    AnalogPort_1 = 1, AnalogPort_2, AnalogPort_3, AnalogPort_4, AnalogPort_5,
    AnalogPort_6,     AnalogPort_7, AnalogPort_8
} AnalogPort;

#define PWM_PORT_COUNT      10
typedef enum PWMPort {
    PWMPort_1 = 1, PWMPort_2, PWMPort_3, PWMPort_4, PWMPort_5,
    PWMPort_6,     PWMPort_7, PWMPort_8, PWMPort_9, PWMPort_10
} PWMPort;

#define UART_PORT_COUNT     2
typedef enum UARTPort {
    UARTPort_1 = 1, UARTPort_2
} UARTPort;

#define I2C_DEVICE_COUNT    10
typedef enum {
    I2c_1 = 1, I2c_2, I2c_3, I2c_4, I2c_5,
    I2c_6,     I2c_7, I2c_8, I2c_9, I2c_10
} I2c;

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

String          Device_getName(Device* device);
DeviceType      Device_getType(Device* device);
String          Device_getTypeName(Device* device);
Device*         Device_getDigitalDevice(DigitalPort port);
DigitalPortMode Device_getDigitalPortMode(DigitalPort port);
Device*         Device_getAnalogDevice(AnalogPort port);
Device*         Device_getPWMDevice(PWMPort port);
PowerExpander*  Device_getPWMExpander(PWMPort port);
Device*         Device_getUARTDevice(UARTPort port);
Device*         Device_getI2cDevice(I2c i2c);
const List*     Device_getDeviceList();
Device*         Device_getByType(DeviceType type);

/********************************************************************
 * Public API: DigitalIn                                            *
 ********************************************************************/

typedef enum {
    InterruptMode_Disabled,
    InterruptMode_RisingEdge,
    InterruptMode_FallingEdge
} InterruptMode;

DigitalIn*    DigitalIn_newBump(String name, DigitalPort port);
DigitalIn*    DigitalIn_newLimit(String name, DigitalPort port);
DigitalIn*    DigitalIn_newJumper(String name, DigitalPort port);
DigitalPort   DigitalIn_getPort(DigitalIn* in);
bool          DigitalIn_isInverted(DigitalIn* in);
void          DigitalIn_setInverted(DigitalIn* in, bool value);
bool          DigitalIn_get(DigitalIn* in);
InterruptMode DigitalIn_getInterruptMode(DigitalIn* in);
void          DigitalIn_setInterruptMode(DigitalIn* in, InterruptMode mode);
bool          DigitalIn_getInterrupted(DigitalIn* in);
Button*       DigitalIn_getButton(DigitalIn* in);

/********************************************************************
 * Public API: Encoder                                              *
 ********************************************************************/

Encoder*    Encoder_newQuadrature(String name, DigitalPort port1, DigitalPort port, bool reversed);
Encoder*    Encoder_new(String name, DigitalPort port);
DigitalPort Encoder_getPort(Encoder* encoder);
DigitalPort Encoder_getPort2(Encoder* encoder);
bool        Encoder_isReversed(Encoder* encoder);
bool        Encoder_isEnabled(Encoder* encoder);
void        Encoder_setEnabled(Encoder* encoder, bool value);
float       Encoder_getScaleFactor(Encoder* encoder);
void        Encoder_setScaleFactor(Encoder* encoder, float scale);
long        Encoder_getRaw(Encoder* encoder);
void        Encoder_presetRaw(Encoder* encoder, long value);
float       Encoder_get(Encoder* encoder);
void        Encoder_preset(Encoder* encoder, float value);

/********************************************************************
 * Public API: Sonar                                                *
 ********************************************************************/

Sonar* Sonar_new(String name, DigitalPort inputPort, DigitalPort outputPort);
DigitalPort Sonar_getInputPort(Sonar* sonar);
DigitalPort Sonar_getOutputPort(Sonar* sonar);
bool   Sonar_isEnabled(Sonar* sonar);
void   Sonar_setEnabled(Sonar* sonar, bool value);
int    Sonar_getDistanceInches(Sonar* sonar);
float  Sonar_getDistanceCentimeters(Sonar* sonar);

/********************************************************************
 * Public API: AnalogIn                                             *
 ********************************************************************/

#define MAX_ANALOG_IN_RAW   4096

AnalogIn*  AnalogIn_newPotentiometer(String name, AnalogPort port);
AnalogIn*  AnalogIn_newLineFollower(String name, AnalogPort port);
AnalogIn*  AnalogIn_newLightSensor(String name, AnalogPort port);
AnalogPort AnalogIn_getPort(AnalogIn* in);
float      AnalogIn_getScaleFactor(AnalogIn* in);
void       AnalogIn_setScaleFactor(AnalogIn* in, float scale);
int        AnalogIn_getRaw(AnalogIn* in);
void       AnalogIn_presetRaw(AnalogIn* in, int value);
float      AnalogIn_get(AnalogIn* in);
void       AnalogIn_preset(AnalogIn* in, float value);

/********************************************************************
 * Public API: Gyro                                                 *
 ********************************************************************/

#define GYRO_DEFAULT_DEADBAND 3

Gyro*      Gyro_new(String name, AnalogPort port);
AnalogPort Gyro_getPort(Gyro* gyro);
void       Gyro_init(Gyro* gyro);
bool       Gyro_isEnabled(Gyro* gyro);
void       Gyro_setEnabled(Gyro* gyro, bool value);
int        Gyro_getDeadband(Gyro* gyro);
void       Gyro_setDeadband(Gyro* gyro, int value);
float      Gyro_getAngleDegrees(Gyro * gyro);

/********************************************************************
 * Public API: Accelerometer                                        *
 ********************************************************************/

Accelerometer* Accelerometer_new(String name, AnalogPort port);
AnalogPort     Accelerometer_getPort(Accelerometer* accel);
void           Accelerometer_init(Accelerometer* accel);
bool           Accelerometer_isEnabled(Accelerometer* accel);
void           Accelerometer_setEnabled(Accelerometer* accel, bool value);
float          Accelerometer_getAccelerationG(Accelerometer* accel);

/********************************************************************
 * Public API: DigitalOut                                           *
 ********************************************************************/

DigitalOut* DigitalOut_newPneumaticValve(String name, DigitalPort port);
DigitalOut* DigitalOut_newLED(String name, DigitalPort port);
DigitalPort DigitalOut_getPort(DigitalOut* out);
bool        DigitalOut_get(DigitalOut* out);
void        DigitalOut_set(DigitalOut* out, bool value);

/********************************************************************
 * Public API: Motor                                                *
 ********************************************************************/

typedef enum {
    MotorType_None,
    MotorType_269    = 269,
    MotorType_393_HT = 393,
    MotorType_393_HS = 394
} MotorType;

PWMPort     Motor_getPort(Motor* motor);
MotorType   Motor_getMotorType(Motor* motor);
I2c         Motor_getI2c(Motor* motor);
MotorGroup* Motor_getGroup(Motor* motor);
bool        Motor_isReversed(Motor* motor);

/********************************************************************
 * Public API: MotorGroup                                           *
 ********************************************************************/

typedef enum {
    FeedbackType_None,
    FeedbackType_IME,
    FeedbackType_Encoder,
    FeedbackType_Potentiometer
} FeedbackType;

MotorGroup* MotorGroup_new(String name);
void        MotorGroup_add(MotorGroup* group, String name, PWMPort port, MotorType type, bool reversed);
void        MotorGroup_addWithIME(MotorGroup* group, String name, PWMPort port, MotorType type, bool reversed, I2c i2c);
const List* MotorGroup_getMotorList(MotorGroup* group);

// open loop control //
Power      MotorGroup_getPower(MotorGroup* group);
Power      MotorGroup_getActualPower(MotorGroup* group);
void       MotorGroup_setPower(MotorGroup* group, Power power);
void       MotorGroup_getPowerRange(MotorGroup* group, Power* min, Power* max);
void       MotorGroup_setPowerRange(MotorGroup* group, Power min, Power max);
void       MotorGroup_getDeadband(MotorGroup* group, Power* min, Power* max);
void       MotorGroup_setDeadband(MotorGroup* group, Power min, Power max);
Power      MotorGroup_getSlewRate(MotorGroup* group);
void       MotorGroup_setSlewRate(MotorGroup* group, Power incrementPerCycle);
DigitalIn* MotorGroup_getReverseLimitSwitch(MotorGroup* group);
void       MotorGroup_setReverseLimitSwitch(MotorGroup* group, DigitalIn* input);
bool       MotorGroup_isReverseLimitOK(MotorGroup* group);
DigitalIn* MotorGroup_getForwardLimitSwitch(MotorGroup* group);
void       MotorGroup_setForwardLimitSwitch(MotorGroup* group, DigitalIn* input);
bool       MotorGroup_isForwardLimitOK(MotorGroup* group);

// feedback monitoring //
void         MotorGroup_addEncoder(MotorGroup* group, Encoder* encoder);
void         MotorGroup_addPotentiometer(MotorGroup* group, AnalogIn* pot);
Device*      MotorGroup_getSensor(MotorGroup* group);
FeedbackType MotorGroup_getFeedbackType(MotorGroup* group);
bool         MotorGroup_isFeedbackEnabled(MotorGroup* group);
void         MotorGroup_setFeedbackEnabled(MotorGroup* group, bool value);
float        MotorGroup_getOutputScaleFactor(MotorGroup* group);
void         MotorGroup_setOutputScaleFactor(MotorGroup* group, float scale);
float        MotorGroup_getFeedbackScaleFactor(MotorGroup* group);
void         MotorGroup_setFeedbackScaleFactor(MotorGroup* group, float scale);
float        MotorGroup_getPosition(MotorGroup* group);
void         MotorGroup_presetPosition(MotorGroup* group, float value);
float        MotorGroup_getSpeed(MotorGroup* group);
void         MotorGroup_restorePosition(MotorGroup* group);

// closed loop control //
bool    MotorGroup_isPIDEnabled(MotorGroup* group);
void    MotorGroup_setPIDEnabled(MotorGroup* group, bool value);
void    MotorGroup_setPID(MotorGroup* group, float kP, float kI, float kD);
float   MotorGroup_getP(MotorGroup* group);
float   MotorGroup_getI(MotorGroup* group);
float   MotorGroup_getD(MotorGroup* group);
float   MotorGroup_getError(MotorGroup* group);
float   MotorGroup_getTolerance(MotorGroup* group);
void    MotorGroup_setTolerance(MotorGroup* group, float tolerance);
bool    MotorGroup_onTarget(MotorGroup* group);
float   MotorGroup_getSetpoint(MotorGroup* group);
void    MotorGroup_setSetpoint(MotorGroup* group, float value);

/********************************************************************
 * Public API: Servo                                                *
 ********************************************************************/

#define SERVO_TRAVEL_DEGREES 100

Servo*  Servo_new(String name, PWMPort port);
PWMPort Servo_getPort(Servo* servo);
float   Servo_getAngleDegrees(Servo* servo);
void    Servo_setAngleDegrees(Servo* servo, float angle);

/********************************************************************
 * Public API: PowerExpander                                        *
 ********************************************************************/

typedef enum {
    PowerExpanderType_Rev_A1 = 456,
    PowerExpanderType_Rev_A2 = 700
} PowerExpanderType;

PowerExpander*    PowerExpander_new(String name, PowerExpanderType type, AnalogPort port);
PowerExpanderType PowerExpander_getType(PowerExpander* expand);
AnalogPort        PowerExpander_getStatusPort(PowerExpander* expand);
void              PowerExpander_setPWMPorts(PowerExpander* expand, PWMPort port1, PWMPort port2, PWMPort port3, PWMPort port4);
void              PowerExpander_getPWMPorts(PowerExpander* expand, PWMPort* port1, PWMPort* port2, PWMPort* port3, PWMPort* port4);
float             PowerExpander_getBatteryVoltage(PowerExpander* expand);

/********************************************************************
 * Public API: LCD                                                  *
 ********************************************************************/

// note: there are more LCD methods in UserInterface.h //
LCD*     LCD_new(String name, UARTPort port);
UARTPort LCD_getPort(LCD* lcd);
void     LCD_setBacklight(LCD* lcd, bool value);
bool     LCD_getBacklight(LCD* lcd);

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

SerialPort*   SerialPort_new(String name, UARTPort port);
void          SerialPort_open(SerialPort* serial, BaudRate baud);
void          SerialPort_openWithOptions(SerialPort* serial, BaudRate baud, SerialOptions opts);
BaudRate      SerialPort_getBaudRate(SerialPort* serial);
SerialOptions SerialPort_getOptions(SerialPort* serial);
unsigned char SerialPort_getByteCount(SerialPort* serial);
unsigned char SerialPort_readByte(SerialPort* serial);
void          SerialPort_writeByte(SerialPort* serial, unsigned char byte);

#endif // _Hardware_h

