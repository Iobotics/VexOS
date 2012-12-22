//
//  robot.h
//  VexOS
//
//  Created by Jeff Malins on 12/5/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#ifndef _Robot_h
#define _Robot_h

#include "VexOS.h"
#include "Hardware.h"

#define MOTOR_DRIVE_L_IME   PWMPort_1
#define MOTOR_DRIVE_L_1     PWMPort_2
#define MOTOR_INTAKE_L      PWMPort_3
#define MOTOR_INTAKE_R      PWMPort_4
#define MOTOR_LIFT_L        PWMPort_5
#define MOTOR_LIFT_R        PWMPort_6
#define MOTOR_DRIVE_L_2     PWMPort_7
#define MOTOR_DRIVE_R_1     PWMPort_8
#define MOTOR_DRIVE_R_2     PWMPort_9
#define MOTOR_DRIVE_R_IME   PWMPort_10

// define digital ports //
#define DIGITAL_LIFT_RESET  DigitalPort_1
#define DIGITAL_PIVOT_VALVE DigitalPort_2

// define analog ports //
#define ANALOG_EXPANDER     AnalogPort_1

// define UART ports //
#define LCD_PORT            UARTPort_1

// command class declarations //
CommandHeader(DriveWithJoystick);
CommandHeader(AutoDrive);
CommandHeader(SetIntake);
CommandHeader(PivotSet);
typedef enum {
    LiftJogDirection_Up,
    LiftJogDirection_Down
} LiftJogDirection;
CommandHeader(LiftJog);
CommandHeader(GroupTest);

// drive subsystem //
SubsystemHeader(Drive);
void Drive_setPower(Power left, Power right);

// intake subsystem //
SubsystemHeader(Intake);
typedef enum {
    IntakeDirection_Stop,
    IntakeDirection_Suck,
    IntakeDirection_Blow
} IntakeDirection;
IntakeDirection Intake_getDirection();
void Intake_setDirection(IntakeDirection dir);

SubsystemHeader(Lift);
typedef enum {
    LiftPosition_Ground   = 0,
    LiftPosition_Descore  = 60,
    LiftPosition_Score    = 90,
    LiftPosition_Free     = -1
} LiftPosition;
void Lift_setPosition(LiftPosition pos);
LiftPosition Lift_getPosition();
void Lift_setPower(Power power);
void Lift_resetEncoders();
bool Lift_getResetSwitch();
bool Lift_getOnTarget();

SubsystemHeader(Pivot);
typedef enum {
    PivotPosition_Down = 5,
    PivotPosition_Up
} PivotPosition;
void Pivot_setPosition(PivotPosition pos);
PivotPosition Pivot_getPosition();

extern Joystick* joystick;

#endif


