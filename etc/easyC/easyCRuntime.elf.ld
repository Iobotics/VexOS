STARTUP("Startup.o")

INPUT("easyCRuntimeLib.lib")

EXTERN( __io_putchar )
EXTERN( _write )

GROUP(
"STM32x_io_putchar_thumb.a"
"std_sbrk_thumb.lib"
"e_stdio_thumb.a"
"libc.a"
"libm.a"
"VexOS.lib"
)

OUTPUT("easyCRuntime.elf")

INCLUDE "STM32F103_384K_64K_FLASH.ld"
