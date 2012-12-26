VexOS for Vex Cortex
=====

An object-encapsulated development environment for the Vex Cortex Microcontroller based on WPILib for FRC (requires the easyC runtime).

How to build / install:
 1. Install "easyC V4 for Cortex", release 4.1.0.5, a demo version should be ok. This is all I have tested so far. Better version detection will be coming shortly.
 2. Add "C:\Program Files (x86)\Intelitek\easyC V4 for Cortex\\#Tools\arm-gcc\bin" to you windows PATH.  
    note: It may be just "C:\Program Files\..." if you are running 32-bit Windows
 3. Set environment variable ROBOTPROJECT equal to your easyC robot project directory.  
    example: set ROBOTPROJECT="c:\Users\jmalins\Documents\Intelitek\easyC V4 for Cortex\Projects\VexOS Robot", where "VexOS Robot" is the name of the robot project (substitute your own project name).
 4. To build and install the VexOS library, in your VexOS repository root (the directory with this file), run 'cs-make project-install' from MS-DOS.
 5. To just build, but not install, simply run 'cs-make'.
 6. In easyC, go to the 'Library Files' section of project explorer, choose 'Add Existing' and pick 'VexOS.lib', which will already be in the project directory.
 7. In the 'Header Files' section, add 'VexOS.h', 'CommandClass.h' and 'Subsystem.h' to your project also.
 8. From there, use your l33t skills to figure out what to do... or wait for more documentation shortly.
