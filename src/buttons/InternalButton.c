//
//  InternalButton.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "ButtonClass.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareButtonClass(InternalButton, {
    bool state;
});

static void constructor(va_list argp) {
    self->fields->state = false;
}

static bool get() {
    return self->fields->state;
}

/********************************************************************
 * Public API                                                       *
 ********************************************************************/

void InternalButton_set(Button* button, bool value) {
    if(button->class != &InternalButton) return;
    button->fields->state = value;
}
