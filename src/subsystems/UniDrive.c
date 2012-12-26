
#include "Subsystem.h"

extern void UniDrive_configure(Subsystem*);

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DefineSubsystem(UniDrive);

static void constructor() {
    UniDrive_configure(self);
}

static void initDefaultCommand() {

}

