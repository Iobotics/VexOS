//
//  Robot.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Hardware.h"
#include "Robot.h"
#include "API.h"

DeclareRobot(&Drive, &Lift, &Intake, &Pivot);

Joystick* joystick;
static PowerExpander* expander;
static LCD* lcd;

static void constructor() {
    joystick = Joystick_new(1);
    expander = PowerExpander_new("Power Expander", PowerExpanderType_Rev_A1, AnalogPort_1);
    lcd      = LCD_new("Main LCD", UARTPort_1);
}

static void initialize() {
    VexOS_setProgramName("My test program of DOOM!");
    InitIntegratedMotorEncoders();
    
    Button* b = Joystick_getButton(joystick, JoystickHand_Left, JoystickButtonType_Up);
    Button_whileHeld(b, Command_new(&AutoDrive));
    
    Button* b3 = Joystick_getButton(joystick, JoystickHand_Left, JoystickButtonType_Left);
    Button_whileHeld(b3, Command_new(&SetIntake, IntakeDirection_Suck));
    Button* b4 = Joystick_getButton(joystick, JoystickHand_Left, JoystickButtonType_Right);
    Button_whileHeld(b4, Command_new(&SetIntake, IntakeDirection_Blow));
    
    Button* b5 = Joystick_getButton(joystick, JoystickHand_Left, JoystickButtonType_Down);
    Button_whileToggled(b5, Command_new(&PivotSet, PivotPosition_Up));
    Button_setToggleGroup(b5, 1);
    Button* b6 = Joystick_getButton(joystick, JoystickHand_Right, JoystickButtonType_Down);
    Button_whileToggled(b6, Command_new(&LiftJog, LiftJogDirection_Up));
    Button_setToggleGroup(b6, 2);
    Button* b7 = Joystick_getButton(joystick, JoystickHand_Right, JoystickButtonType_Up);
    Button_whileToggled(b7, Command_new(&LiftJog, LiftJogDirection_Down));
    Button_setToggleGroup(b7, 2);
    

    VexOS_setupStandardUI();
    
    PowerScaler* ps = PowerScaler_new("Sarah Scale");
    PowerScaler_addPoint(ps, 0.6, 0.3);
    PowerScaler_addPoint(ps, 0.4, 0.1);
    PrintToScreen(PowerScaler_toString(ps));
    
    Autonomous_addProgram(Command_new(&WaitCommand, 3000));
    Command* kw = Command_new(&GroupTest);
    Autonomous_addProgram(kw);
    Autonomous_addProgram(Command_new(&CommandGroup, "Self Destruct"));
    Button* b8 = Joystick_getButton(joystick, JoystickHand_Right, JoystickButtonType_Right);
    Button_whenPressed(b8, kw);

    //Autonomous_setSelectedProgramByNumber(2);
    Autonomous_restoreLastProgram();
#ifdef XCODE
    VexOS_addEventHandler(EventType_AutonomousPeriodic, &autoPeriodic, NULL);
#endif
}





