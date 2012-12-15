#ifndef __GLOBAL_DATA_H
#define __GLOBAL_DATA_H

#define MAX_EASYC_GLOBALS 20
extern unsigned long g_ulGlobalData[];
#define easyCGlobalData(C) (g_ulGlobalData[C < 1 || C > MAX_EASYC_GLOBALS ? 0 : C])

#endif