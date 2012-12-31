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
    CommandGroup_addSequentialWithTimeout(self, Command_new(&WaitCommand, 5.5), 2.8);
    CommandGroup_addParallel(self, Command_new(&WaitCommand, 5.0));
    CommandGroup_addParallel(self, Command_new(&WaitCommand, 3.0));
    CommandGroup_addParallel(self, Command_new(&WaitUntilCommand, 13.0));
    CommandGroup_addParallel(self, Command_new(&StartCommand, Command_new(&WaitUntilCommand, 25.0)));
    CommandGroup_addSequential(self, Command_new(&WaitForChildren));
    CommandGroup_addSequential(self, Command_new(&PrintCommand, "Hello Yeah!"));
    CommandGroup_addSequential(self, Command_new(&WaitCommand, 7.0));

    setName("Will Kaialua");
}




