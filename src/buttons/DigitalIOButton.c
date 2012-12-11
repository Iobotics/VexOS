//
//  DigitalIOButton.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "ButtonClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareButtonClass(DigitalIOButton, {
    DigitalPort port;
});

static void constructor(va_list argp) {
    self->fields->port = va_arg(argp, int);
    setArgs("%d", self->fields->port);
}

static bool get() {
    return !GetDigitalInput(self->fields->port);
}

const ButtonClass DigitialIOButton = {
    .name        = "DigitialIOButton",
    .constructor = &constructor,
    .get         = &get
};
