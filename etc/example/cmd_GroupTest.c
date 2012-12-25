//
//  cmd_GroupTest.c
//  VexOS
//
//  Created by Jeff Malins on 12/19/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineCommandGroup(GroupTest);

static void constructor(va_list argp) {
    CommandGroup_addSequentialWithTimeout(self, Command_new(&WaitCommand, 5500), 2800);
    CommandGroup_addParallel(self, Command_new(&WaitCommand, 5000));
    CommandGroup_addParallel(self, Command_new(&WaitCommand, 3000));
    CommandGroup_addParallel(self, Command_new(&WaitUntilCommand, 13000));
    CommandGroup_addParallel(self, Command_new(&StartCommand, Command_new(&WaitUntilCommand, 25000)));
    CommandGroup_addSequential(self, Command_new(&WaitForChildren));
    CommandGroup_addSequential(self, Command_new(&PrintCommand, "Hello Yeah!"));
    CommandGroup_addSequential(self, Command_new(&WaitCommand, 7000));

    setName("Will Kaialua");
}



