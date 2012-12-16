##
##  Makefile
##  VexOS for Vex Cortex
##
##  Created by Jeff Malins on 12/10/2012.
##  Copyright (c) 2012 Jeff Malins. All rights reserved.
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU Lesser General Public License as published 
##  by the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
##

# project paths #
SRCDIR    := src
ETCDIR    := etc
INCPUBDIR := include
INCINTDIR := headers
OBJDIR    := build

# GCC-ARM toolchain #
TOOLS    := "C:\Program Files (x86)\Intelitek\easyC V4 for Cortex\\\#TOOLS"
TOOLSBIN := $(TOOLS)\arm-gcc\bin
STEMDIR  := $(TOOLS)\STM32F103VD
CC := $(TOOLSBIN)\arm-none-eabi-gcc.exe
AR := $(TOOLSBIN)\arm-none-eabi-ar.exe
INCLUDES := -I $(INCPUBDIR) -I $(INCINTDIR) -I $(INCINTDIR)\hardware -I $(INCINTDIR)\ui \
			-I $(STEMDIR)\inc -I $(TOOLS)\API
CFLAGS   := -mthumb -D DEBUG -O3 -Wall -c

# objects #
OS_OBJS  := Autonomous.o Battery.o Button.o ButtonClass.o Command.o CommandClass.o \
			CommandGroup.o DebugValue.o Error.o List.o PIDController.o PowerScaler.o \
			Scheduler.o Subsystem.o Timer.o VexOS.o
CMD_OBJS := PrintCommand.o StartCommand.o WaitCommand.o WaitForChildren.o WaitUntilCommand.o
BTN_OBJS := JoystickButton.o DigitalIOButton.o InternalButton.o
HDW_OBJS := Accelerometer.o AnalogIn.o Device.o DigitalIn.o DigitalOut.o Encoder.o Gyro.o \
			Motor.o MotorGroup.o PowerExpander.o SerialPort.o Servo.o Sonar.o 
UI_OBJS  := Dashboard.o LCD.o LCDScreen.o Status.o Window.o
SYS_OBJS := UniDrive.o
ALL_OBJS := $(OS_OBJS) $(CMD_OBJS) $(BTN_OBJS) $(HDW_OBJS) $(UI_OBJS) $(SYS_OBJS)

# make everything to the build directory #
vpath %.h $(INCPUBDIR);$(INCINTDIR);$(INCINTDIR)/hardware;$(INCINTDIR)/ui
vpath %.c $(SRCDIR);$(SRCDIR)/buttons;$(SRCDIR)/commands;$(SRCDIR)/hardware;$(SRCDIR)/ui
vpath %.c $(SRCDIR)/subsystems
OBJS := $(addprefix $(OBJDIR)/,$(ALL_OBJS))

# everything is combined into VexOS.lib #
$(OBJDIR)/VexOS.lib : $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(OBJDIR)/%.o : %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OUTPUT_OPTION) $<
$(OBJS): | $(OBJDIR) 
$(OBJDIR):
	mkdir $(OBJDIR)

# code-to-header dependencies #
$(OBJDIR)/%.o : %.h
$(OBJDIR)/Button.o : ButtonClass.h Scheduler.h
$(OBJDIR)/Command.o : CommandClass.h CommandGroup.h Scheduler.h 
$(OBJDIR)/CommandGroup.o : Command.h CommandClass.h
$(OBJDIR)/Dashboard.o : Window.h
$(OBJDIR)/DebugValue.o : LCDScreen.h
$(OBJDIR)/LCD.o : LCDScreen.h
$(OBJDIR)/Scheduler.o : Button.h Command.h Subsystem.h
$(OBJDIR)/VexOS.o : Error.h Scheduler.h
$(OBJS) : VexOS.h Error.h

# built-in objects depend on their class headers #
$(addprefix $(OBJDIR)/,$(BTN_OBJS)) : ButtonClass.h
$(addprefix $(OBJDIR)/,$(CMD_OBJS)) : CommandClass.h
$(addprefix $(OBJDIR)/,$(UI_OBJS))  : UserInterface.h
$(addprefix $(OBJDIR)/,$(HDW_OBJS)) : Device.h Hardware.h

# install into external easyC project directory #
.PHONY : project-install
project-install: $(OBJDIR)/VexOS.lib
ifdef ROBOTPROJECT
	cp $(INCPUBDIR)\*.h $(ROBOTPROJECT)
	cp $(OBJDIR)\VexOS.lib $(ROBOTPROJECT)
else
	$(error ROBOTPROJECT environment variable is not defined)
endif

# install into easyC example directory #
.PHONY : example
example: $(OBJDIR)/VexOS.lib
	cp $(INCPUBDIR)/*.h $(ETCDIR)/example
	cp $(OBJDIR)/VexOS.lib $(ETCDIR)/example

.PHONY : example-clean
example-clean: 
	rm -f $(ETCDIR)/example/VexOS.lib
	rm -f $(ETCDIR)/example/ButtonClass.h
	rm -f $(ETCDIR)/example/CommandClass.h
	rm -f $(ETCDIR)/example/Hardware.h
	rm -f $(ETCDIR)/example/Subsystem.h
	rm -f $(ETCDIR)/example/VexOS.h
	rm -f $(ETCDIR)/example/VexOS_Robot.HEX

# install into the easyC global tools directories #
.PHONY : global-install
global-install: $(OBJDIR)/VexOS.lib
	cp $(INCPUBDIR)\*.h $(STEMDIR)\inc
	cp $(OBJDIR)\VexOS.lib $(STEMDIR)\Link
	mv $(STEMDIR)\Scripts\easyCRuntime.elf.ld $(STEMDIR)\Scripts\easyCRuntime.elf.ld.old
	cp $(ETCDIR)\easyCRuntime.elf.ld $(STEMDIR)\Scripts

# uninstall from the easyC global tools directories #
.PHONY : global-uninstall
global-uninstall: 
	rm -f $(STEMDIR)\inc\ButtonClass.h
	rm -f $(STEMDIR)\inc\CommandClass.h
	rm -f $(STEMDIR)\inc\Subsystem.h
	rm -f $(STEMDIR)\inc\VexOS.h
	rm -f $(STEMDIR)\Link\VexOS.lib
	mv $(STEMDIR)\Scripts\easyCRuntime.elf.ld $(STEMDIR)\Scripts\easyCRuntime.elf.ld.bak
	mv $(STEMDIR)\Scripts\easyCRuntime.elf.ld.old $(STEMDIR)\Scripts\easyCRuntime.elf.ld

# clean up everything #
.PHONY : clean
clean:
	rm -Rf $(OBJDIR)
