//
//  main.c
//  VexOS
//
//  Created by Jeff Malins on 11/21/12.
//  Copyright (c) 2012 Jeff Malins. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include "API.h"
#include "VexOS.h"

void VexOS_Initialize();
void VexOS_OperatorControl();
void VexOS_Autonomous();

double StartTime;
double getTimeMs() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec + (t.tv_usec / 1000000.0)) * 1000.0;
}

void autoPeriodic(EventType type, void* state) {
    Wait(100);
    PrintToScreen("  time: %d\n", GetMsClock());
}

void operatorPeriodic(EventType type, void* state) {
    Wait(100);
    PrintToScreen("  time: %d\n", GetMsClock());
}

int main(int argc, const char* argv[]) {
    GlobalData(GLOBALDATA_AUTO_PROGRAM) = 2;
    GlobalData(GLOBALDATA_DASH_NUMBER)  = 1;
    
    printf("Hello, World!\n");
    StartTime = getTimeMs();
    VexOS_Initialize();
    VexOS_OperatorControl();
    //VexOS_Autonomous();
    return 0;
}