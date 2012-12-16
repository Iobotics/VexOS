//
//  Robot.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "Hardware.h"
#include "Robot.h"

RobotSubsystems(&Drive, &Lift, &Intake, &Pivot);

void autoPeriodic(EventType type) {
    Wait(100);
    PrintToScreen("  time: %d\n", GetMsClock());
}

Command* getAutonomousKW() {
    Command* group = Command_new(&CommandGroup, "Will Kailua");
    CommandGroup_addSequentialWithTimeout(group, Command_new(&WaitCommand, 550), 280);
    CommandGroup_addParallel(group, Command_new(&WaitCommand, 500));
    CommandGroup_addParallel(group, Command_new(&WaitCommand, 300));
    CommandGroup_addParallel(group, Command_new(&WaitUntilCommand, 1300));
    CommandGroup_addParallel(group, Command_new(&StartCommand, Command_new(&WaitUntilCommand, 2500)));
    CommandGroup_addSequential(group, Command_new(&WaitForChildren));
    CommandGroup_addSequential(group, Command_new(&PrintCommand, "Hello Yeah!"));
    CommandGroup_addSequential(group, Command_new(&WaitCommand, 700));
    return group;
}

void InitializeRobot() {
    VexOS_setProgramName("My test program of DOOM!");
    InitIntegratedMotorEncoders();
    
    //Subsystem_register(&Drive);
    //Subsystem_register(&Lift);
    //Subsystem_register(&Intake);
    //Subsystem_register(&Pivot);
    //Subsystem_register(NULL);
    
    Button* b = Button_new(&JoystickButton, 1, 7, 2);
    Button_whileHeld(b, Command_new(&AutoDrive));
    
    Button* b3 = Button_new(&JoystickButton, 1, 7, 3);
    Button_whenPressed(b3, Command_new(&SetIntake, IntakeDirection_Suck));
    Button* b4 = Button_new(&JoystickButton, 1, 7, 4);
    Button_whenPressed(b4, Command_new(&SetIntake, IntakeDirection_Blow));
    
    Button* b5 = Button_new(&JoystickButton, 1, 7, 1);
    Button_whileToggled(b5, Command_new(&PivotSet, PivotPosition_Up));
    Button_setToggleGroup(b5, 1);
    Button* b6 = Button_new(&JoystickButton, 1, 8, 1);
    Button_whileToggled(b6, Command_new(&LiftJog, LiftJogDirection_Up));
    Button_setToggleGroup(b6, 2);
    Button* b7 = Button_new(&JoystickButton, 1, 8, 2);
    Button_whileToggled(b7, Command_new(&LiftJog, LiftJogDirection_Down));
    Button_setToggleGroup(b7, 2);
    
    //SetPowerExpander(AnalogPort_1, PowerExpanderType_Rev_A1);
    LCD_setPort(UARTPort_1);
    
    PowerScaler* ps = PowerScaler_new("Sarah Scale");
    PowerScaler_addPoint(ps, 0.6, 0.3);
    PowerScaler_addPoint(ps, 0.4, 0.1);
    PrintToScreen(PowerScaler_toString(ps));
    
    VexOS_addAutonomous(Command_new(&WaitCommand, 3000));
    VexOS_addAutonomous(getAutonomousKW());
    VexOS_addAutonomous(Command_new(&CommandGroup, "Self Destruct"));
    Dashboard_setEnabled(true);
    
    VexOS_setSelectedAutonomousByNumber(2);
    VexOS_addEventHandler(EventType_AutonomousPeriodic, autoPeriodic);
}


