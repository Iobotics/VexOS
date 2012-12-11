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
INCLUDES := -I $(INCPUBDIR) -I $(INCINTDIR) -I $(STEMDIR)\inc -I $(TOOLS)\API
CFLAGS   := -mthumb -D DEBUG -O3 -c

# objects #
OS_OBJS  := Button.o Command.o CommandGroup.o  Dashboard.o DebugValue.o Error.o LCD.o \
			LCDScreen.o List.o PIDController.o PowerScaler.o Scheduler.o Subsystem.o \
			VexOS.o Window.o
CMD_OBJS := PrintCommand.o StartCommand.o WaitCommand.o WaitForChildren.o WaitUntilCommand.o
BTN_OBJS := JoystickButton.o DigitalIOButton.o InternalButton.o
ALL_OBJS := $(OS_OBJS) $(CMD_OBJS) $(BTN_OBJS)

# make everything to the build directory #
vpath %.h $(INCPUBDIR);$(INCINTDIR)
vpath %.c $(SRCDIR);$(SRCDIR)/buttons;$(SRCDIR)/commands
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
$(OBJDIR)/DebugValue.o : LCD.h LCDScreen.h Window.h
$(OBJDIR)/Error.o : LCD.h
$(OBJDIR)/LCD.o : LCDScreen.h
$(OBJDIR)/Scheduler.o : Button.h Command.h Subsystem.h Window.h
$(OBJDIR)/VexOS.o : Dashboard.h Error.h LCD.h Scheduler.h Window.h
$(OBJS) : VexOS.h Error.h

# built-in objects depend on their class headers #
$(addprefix $(OBJDIR)/,$(BTN_OBJS)) : ButtonClass.h
$(addprefix $(OBJDIR)/,$(CMD_OBJS)) : CommandClass.h

# install into easyC project directory #
.PHONY : project-install
project-install: $(OBJDIR)/VexOS.lib
ifdef ROBOTPROJECT
	cp $(INCPUBDIR)\*.h $(ROBOTPROJECT)
	cp $(OBJDIR)\VexOS.lib $(ROBOTPROJECT)
else
	$(error ROBOTPROJECT environment variable is not defined)
endif

# install into the easy global tools directories #
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
