//
//  cmd_LiftJog.c
//  VexOS
//
//  Created by Jeff Malins on 12/6/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include "CommandClass.h"
#include "Robot.h"

/********************************************************************
 * Class Definition                                                 *
 ********************************************************************/

DeclareCommandClass(LiftJog, {
    LiftJogDirection dir;
});

static void constructor(va_list argp) {
    self->fields->dir = (LiftJogDirection) va_arg(argp, int);
    setArgs("%s", (self->fields->dir == LiftJogDirection_Down)? "Down": "Up");
    require(&Lift);
}

static void initialize() {
    LiftPosition pos = Lift_getPosition();
    switch(self->fields->dir) {
        case LiftJogDirection_Up:
            // handle jog up by position //
            switch(pos) {
                case LiftPosition_Ground:  pos = LiftPosition_Descore; break;
                case LiftPosition_Descore: pos = LiftPosition_Score;   break;
                case LiftPosition_Score:   break;
                case LiftPosition_Free:    pos = LiftPosition_Score;   break;
            }
            break;
        case LiftJogDirection_Down:
            // handle jog down by position //
            switch(pos) {
                case LiftPosition_Ground:  break;
                case LiftPosition_Descore: pos = LiftPosition_Ground;  break;
                case LiftPosition_Score:   pos = LiftPosition_Descore; break;
                case LiftPosition_Free:    pos = LiftPosition_Ground;  break;
            }
            break;
    }
    // update lift with new position //
    Lift_setPosition(pos);
}

static void execute() { }

static bool isFinished() {
    return true;
}

static void end() { }

static void interrupted() { }


