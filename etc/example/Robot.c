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

static PowerExpander* expander = NULL;
static LCD* lcd = NULL;

static void constructor() {
    expander = PowerExpander_new("Power Expander", PowerExpanderType_Rev_A1, AnalogPort_1);
    lcd      = LCD_new("Main LCD", UARTPort_1);
}

static void initialize() {
    VexOS_setProgramName("My test program of DOOM!");
    InitIntegratedMotorEncoders();
        
    Button* b = Button_new(&JoystickButton, 1, 7, 2);
    Button_whileHeld(b, Command_new(&AutoDrive));
    
    Button* b3 = Button_new(&JoystickButton, 1, 7, 3);
    Button_whileHeld(b3, Command_new(&SetIntake, IntakeDirection_Suck));
    Button* b4 = Button_new(&JoystickButton, 1, 7, 4);
    Button_whileHeld(b4, Command_new(&SetIntake, IntakeDirection_Blow));
    
    Button* b5 = Button_new(&JoystickButton, 1, 7, 1);
    Button_whileToggled(b5, Command_new(&PivotSet, PivotPosition_Up));
    Button_setToggleGroup(b5, 1);
    Button* b6 = Button_new(&JoystickButton, 1, 8, 1);
    Button_whileToggled(b6, Command_new(&LiftJog, LiftJogDirection_Up));
    Button_setToggleGroup(b6, 2);
    Button* b7 = Button_new(&JoystickButton, 1, 8, 2);
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
    Button* b8 = Button_new(&JoystickButton, 1, 8, 3);
    Button_whenPressed(b8, kw);

    //Autonomous_setSelectedProgramByNumber(2);
    Autonomous_restoreLastProgram();
#ifdef XCODE
    VexOS_addEventHandler(EventType_AutonomousPeriodic, &autoPeriodic, NULL);
#endif
}





