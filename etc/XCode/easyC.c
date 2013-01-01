//
//  easyC.c
//  VexOS
//
//  Created by Jeff Malins on 11/21/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "API.h"

#define INTERRUPT_MILLIS    20
extern double StartTime;
double getTimeMs();

typedef void (Handler)();
static Handler* _handler;

void Wait(long ulTime) {
    unsigned long start   = GetMsClock();
    unsigned long nextISR = 0;
    while((GetMsClock() - start) < ulTime) {
        if(GetMsClock() > nextISR) {
            nextISR += INTERRUPT_MILLIS;
            if(_handler) _handler();
        }
    }
}

unsigned long GetMsClock() {
    return (unsigned long) (getTimeMs() - StartTime);
}

unsigned char IsEnabled(void) {
    return true;
}

void PrintToScreen(const char* msg, ...) {
    va_list argp;
    va_start(argp, msg);
    vprintf(msg, argp);
    va_end(argp);
}

double Fabs(double value) {
    return (value < 0)? -value: value;
}

void RegisterImeInterruptServiceRoutine(Handler* handler) {
    _handler = handler;
}

void UnRegisterImeInterruptServiceRoutine(Handler* handler) {
    if(_handler == handler) _handler = NULL;
}

void SetSaveCompetitionIme(unsigned char ucSave) {
    
}

long GetSavedCompetitionIme(unsigned char ucMotor) {
    return 0;
}

void GetIntegratedMotorEncodersData(ImeData ime[MAX_IME]) {
    for(int i = 0; i < MAX_IME; i++) {
        ime[i].counter = i;
        ime[i].speed   = i * 10;
    }
}

void PrintTextToGD(unsigned char ucRow, unsigned char ucCol, unsigned long ulColor, const char *szText, ...) {
    
}

void PrintFrameToGD(unsigned char ucRow1, unsigned char ucCol1, unsigned char ucRow2, unsigned char ucCol2, unsigned long ulColor) {
    
}

void ClearGD(unsigned char ucRow1, unsigned char ucCol1, unsigned char ucRow2, unsigned char ucCol2, unsigned char ucFrame) {
    
}

void ResetGD() { }

void InitLCD(unsigned char port) {
    
}

unsigned char SetLCDLight(unsigned char port, unsigned char value) {
    return value;
}

void StartLCDButtonsWatcher(unsigned char ucPort) {
    
}

void StopLCDButtonsWatcher(unsigned char ucPort) {
    
}

void GetLCDButtonsWatcher(unsigned char ucPort, unsigned char *b1, unsigned char *b2, unsigned char *b3) {
    
}

unsigned char SetLCDText(unsigned char ucPort, unsigned char nLine, const char *szMsg, ...) {
    return ucPort;
}

void SetMotor(unsigned char ucMotor, int iSpeed) {
    
}
unsigned char GetDigitalInput(unsigned char ucPort) {
    return 1;
}
void SetDigitalOutput(unsigned char ucPort, unsigned char ucValue) {
    
}

void StartTimer(unsigned char ucTimerNumber) {
    
}

void StopTimer(unsigned char ucTimerNumber) {
    
}

void PresetTimer(unsigned char ucTimerNumber, unsigned long ulValue) {
    
}

unsigned long GetTimer(unsigned char ucTimerNumber) {
    return 0;
}

unsigned int GetUltrasonic(unsigned char ucEcho, unsigned char ucPing) {
    return 0;
}

void StartUltrasonic(unsigned char ucEcho, unsigned char ucPing) {

}

void StopUltrasonic(unsigned char ucEcho, unsigned char ucPing) {
    
}

void StartEncoder(unsigned char ucPort) {
    
}
void StopEncoder(unsigned char ucPort) {
    
}
long GetEncoder(unsigned char ucPort) {
    return 0;
}
void PresetEncoder(unsigned char ucPort, long lPresetValue) {
    
}

void StartQuadEncoder(unsigned char ucChannelA, unsigned char ucChannelB, unsigned char ucInvert) {
    
}
void StopQuadEncoder(unsigned char ucChannelA, unsigned char ucChannelB) {
    
}
long GetQuadEncoder(unsigned char ucChannelA, unsigned char ucChannelB) {
    return 0;
}
void PresetQuadEncoder(unsigned char ucChannelA, unsigned char ucChannelB, long lPresetValue) {
    
}

void StartInterruptWatcher(unsigned char port, unsigned char direction) {
    
}
void StopInterruptWatcher(unsigned char port) {
    
}
unsigned char GetInterruptWatcher(unsigned char port) {
    return 0;
}

void InitAccelerometer(unsigned char ucPort) {
    
}
void StartAccelerometer(unsigned char ucPort) {
    
}
int GetAcceleration(unsigned char ucPort) {
    return 0;
}
void StopAccelerometer(unsigned char ucPort) {
    
}

void InitGyro(unsigned char ucPort) {
    
}
void StartGyro(unsigned char ucPort) {
    
}
void StopGyro(unsigned char ucPort) {
    
}
int GetGyroAngle(unsigned char ucPort) {
    return 0;
}
void SetGyroType(unsigned char ucPort, unsigned int nType) {
    
}
void SetGyroDeadband(unsigned char ucPort, char cDeadband) {
    
}
long GetGyroReal(unsigned char port) {
    return 0;
}

void OpenSerialPort(unsigned char ucPort, unsigned long ulBaudRate) {
    
}
void OpenSerialPortEx(unsigned char ucPort, unsigned long ulBaudRate,
                      unsigned short usWordLength,
                      unsigned short usStopBits,
                      unsigned short usParity,
                      unsigned short usHardwareFlowControl,
                      unsigned short usMode) {
    
}
unsigned char ReadSerialPort(unsigned char ucPort) {
    return 0;
}
void WriteSerialPort(unsigned char ucPort, char cData) {
    
}
unsigned char GetSerialPortByteCount(unsigned char ucPort) {
    return 0;
}

void SetServo(unsigned char ucMotor, int iSpeed) {
    
}

unsigned int GetAnalogInput(unsigned char ucPort) {
    return 0;
}
unsigned int GetAnalogInputHR(unsigned char ucPort) {
    return 0;
}


int GetJoystickAnalog(unsigned char ucJoystick, unsigned char ucChannel) {
    return 0;
}

unsigned char GetJoystickDigital(unsigned char ucJoystick, unsigned char ucChannel, unsigned char ucButton) {
    return 0;
}

void DefineIntegratedMotorEncoderPID(unsigned char ucMotor, float fKc, float fTi  , float fTd, long lTolerance) {
    
}
void StartIntegratedMotorEncoderPID(unsigned char ucMotor, long lSetpoint) {
    
}
void UpdateSetpointIntegratedMotorEncoderPID(unsigned char ucMotor, long lSetpoint) {
    
}
unsigned char OnTargetIntegratedMotorEncoderPID(unsigned char ucMotor) {
    return 1;
}
void StopIntegratedMotorEncoderPID(unsigned char ucMotor) {

}
unsigned char InitIntegratedMotorEncoders(void) {
    return 0;
}
void PresetIntegratedMotorEncoder(unsigned char ucMotor, long lPresetValue) {
    
}

float GetMainBattery() {
    return 0.0;
}

float GetBackupBattery() {
    return 0.0;
}

void DefineControllerIO(unsigned char ucP1, unsigned char ucP2, unsigned char ucP3, unsigned char ucP4,
                        unsigned char ucP5, unsigned char ucP6, unsigned char ucP7, unsigned char ucP8,
                        unsigned char ucP9, unsigned char ucP10, unsigned char ucP11, unsigned char ucP12)
{
    PrintToScreen("DefineControllerIO(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                  ucP1, ucP2, ucP3, ucP4, ucP5, ucP6, ucP7, ucP8, ucP9, ucP10, ucP11, ucP12);
}

void DefineImeTable(unsigned char ucMotor1I2C, unsigned char ucMotor2I2C, unsigned char ucMotor3I2C, unsigned char ucMotor4I2C,
                    unsigned char ucMotor5I2C, unsigned char ucMotor6I2C, unsigned char ucMotor7I2C, unsigned char ucMotor8I2C,
                    unsigned char ucMotor9I2C, unsigned char ucMotor10I2C)
{
    PrintToScreen("DefineImeTable(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                  ucMotor1I2C, ucMotor2I2C, ucMotor3I2C, ucMotor4I2C, ucMotor5I2C, ucMotor6I2C, ucMotor7I2C, ucMotor8I2C, ucMotor9I2C, ucMotor10I2C);
}

void DefineMotorTypes(unsigned char ucMotorType1, unsigned char ucMotorType2, unsigned char ucMotorType3, unsigned char ucMotorType4,
                      unsigned char ucMotorType5, unsigned char ucMotorType6, unsigned char ucMotorType7, unsigned char ucMotorType8,
                      unsigned char ucMotorType9, unsigned char ucMotorType10)
{
    PrintToScreen("DefineMotorTypes(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                  ucMotorType1, ucMotorType2, ucMotorType3, ucMotorType4, ucMotorType5, ucMotorType6, ucMotorType7, ucMotorType8, ucMotorType9, ucMotorType10);
}

#define MAX_EASYC_GLOBALS 20
unsigned long g_ulGlobalData[MAX_EASYC_GLOBALS + 1];
#define easyCGlobalData(C) (g_ulGlobalData[C < 1 || C > MAX_EASYC_GLOBALS ? 0 : C])
