/**********************************************************************************************************************
 * \file Cpu0_Main.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include <stdio.h>
#include <string.h>

IfxCpu_syncEvent g_cpuSyncEvent = 0;

volatile int debugX;

unsigned int systemtick[4];

volatile int checksum_0;
volatile int checksum_1;

#define SYSTEM_TIMER_31_0   *(unsigned int *)(0xF0000000+0x10)

int core0_main(void)
{
    IfxCpu_enableInterrupts();
    
    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
    
    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);
    
    printf("Hello World\n");

    // CPU0 Data Scratch-Pad RAM
    systemtick[0] = SYSTEM_TIMER_31_0;
    checksum_0 = 0;
    for( int i=0; i<0x2000; i++)
        checksum_0 += *((volatile int *)0x70008000+i);
    systemtick[1] = SYSTEM_TIMER_31_0;

    // CPU1 Data Scratch-Pad RAM
    systemtick[2] = SYSTEM_TIMER_31_0;
    checksum_1 = 0;
    for( int i=0; i<0x2000; i++)
        checksum_1 += *((volatile int *)0x60008000+i);
    systemtick[3] = SYSTEM_TIMER_31_0;

    printf("0x7000 access @ cpu0 : %d\n", systemtick[1]-systemtick[0]);
    printf("0x6000 access @ cpu0 : %d\n", systemtick[3]-systemtick[2]);

    systemtick[0] = SYSTEM_TIMER_31_0;
    memcpy((char *)0x70008000,(char *)0x70008000, 0x8000);
    systemtick[1] = SYSTEM_TIMER_31_0;

    systemtick[2] = SYSTEM_TIMER_31_0;
    memcpy((char *)0x60008000,(char *)0x60008000, 0x8000);
    systemtick[3] = SYSTEM_TIMER_31_0;

    printf("0x7000 memcpy @ cpu0 : %d\n", systemtick[1]-systemtick[0]);
    printf("0x6000 memcpy @ cpu0 : %d\n", systemtick[3]-systemtick[2]);

    while(1)
    {
    }
    return (1);
}
