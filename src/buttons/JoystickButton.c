//
//  JoystickButton.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "ButtonClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareButtonClass(JoystickButton, {
    unsigned char       joystick;
    JoystickChannelType channel;
    JoystickButtonType  button;
});

static void constructor(va_list argp) {
    self->fields->joystick = va_arg(argp, unsigned int);
    self->fields->channel  = va_arg(argp, int);
    self->fields->button   = va_arg(argp, int);
    setArgs("%d, %d, %d", self->fields->joystick, self->fields->channel, self->fields->button);
}

static bool get() {
    Fields* fields = self->fields;
    return GetJoystickDigital(fields->joystick, fields->channel, fields->button);
}
