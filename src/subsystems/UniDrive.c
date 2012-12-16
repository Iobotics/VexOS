
#include "Subsystem.h"

extern void UniDrive_configure(Subsystem*);

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareSubsystem(UniDrive);

static void constructor() {
    UniDrive_configure(self);
}

static void initDefaultCommand() {

}

