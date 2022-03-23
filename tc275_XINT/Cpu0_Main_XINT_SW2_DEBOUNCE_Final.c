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

#define SYSTEM_TIMER_0_31_0 *(unsigned int *)(0xF0000000+0x10)

/* Define PORT10 Registers for LED */
#define PORT10_BASE     (0xF003B000)
#define PORT10_IOCR0    (*(volatile unsigned int*)(PORT10_BASE + 0x10))
#define PORT10_OMR      (*(volatile unsigned int*)(PORT10_BASE + 0x04))

#define PC0             0
#define PC1             11
#define PC2             19

#define PCL1            17
#define PCL2            18

#define PS1             1
#define PS2             2

/* Define PORT02 Registers for Switch2 */
#define PORT02_BASE     (0xF003A200)
#define PORT02_IOCR0    (*(volatile unsigned int*)(PORT02_BASE + 0x10))
#define PORT02_IN       (*(volatile unsigned int*)(PORT02_BASE + 0x24))


#define P0              0
#define P1              1

/* Define SCU Registers for Interrupt */
#define SCU_BASE        (0xF0036000)
#define SCU_WDTSCON0    (*(volatile unsigned int*)(SCU_BASE + 0x0F0))
#define SCU_EICR1       (*(volatile unsigned int*)(SCU_BASE + 0x214))
#define SCU_IGCR0       (*(volatile unsigned int*)(SCU_BASE + 0x22C))

#define LCK             1
#define ENDINIT         0

#define IGP0            14
#define INP0            12
#define EIEN0           11
#define FEN0            8
#define EXIS0           4

#define IGP1            30
#define INP1            28
#define EIEN1           27
#define FEN1            24
#define EXIS1           20

/* Define SRC Registers for Interrupt */
#define SRC_BASE        (0xF0038000)
#define SRC_SCUERU0     (*(volatile unsigned int*)(SRC_BASE + 0xCD4))
#define SRC_SCUERU1     (*(volatile unsigned int*)(SRC_BASE + 0xCD8))

#define TOS             11
#define SRE             10
#define SRPN            0

IfxCpu_syncEvent g_cpuSyncEvent = 0;

/* Initialize LED (RED) */
void init_LED(void)
{
    /* Reset PC1 & PC2 in IOCR0*/
    PORT10_IOCR0 &= ~((0x1F) << PC1);
    PORT10_IOCR0 &= ~((0x1F) << PC2);

    /* Set PC1 & PC2 with push-pull(2b10000) */
    PORT10_IOCR0 |= ((0x10) << PC1);
    PORT10_IOCR0 |= ((0x10) << PC2);
}

/* Initialize Switch */
void init_Switch(void)
{
    /* Reset PC1 and PC0 in IOCR0*/
    PORT02_IOCR0 &= ~((0x1F) << PC1);
    PORT02_IOCR0 &= ~((0x1F) << PC0);

    /* Set PC1 and PC0 with pull-up(2b0xx10) */
    PORT02_IOCR0 |= ((0x2) << PC1);
    PORT02_IOCR0 |= ((0x2) << PC0);
}

/* Initialize External Request Unit (ERU) */
void init_ERU(void)
{
    /* ERU Input Channel 2 Setting */
    /* Password Access to unlock WDTSCON0 */
    SCU_WDTSCON0 = ((SCU_WDTSCON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDTSCON0 & (1 << LCK)) != 0);

    // Modify Access to clear ENDINIT bit
    SCU_WDTSCON0 = ((SCU_WDTSCON0 ^ 0xFC) | (1 << LCK)) & ~(1 << ENDINIT);
    while((SCU_WDTSCON0 & (1 << LCK)) == 0);

    SCU_EICR1 &= ~(((0x7) << EXIS0) | ((0x7) << EXIS1));    // External input clear
    SCU_EICR1 &= ~(((0x7) << INP0) | ((0x7) << INP1));      // Input Node Pointer Clear
    SCU_IGCR0 &= ~(((0x3) << IGP0) | ((0x3) << IGP1));      // Interrupt Gating Patten 2, 3 Clear

    SCU_EICR1 |= (0x1 << EXIS0)     |                       // P02.1 select
                 (0x2 << EXIS1)     ;                       // P02.0 select

    SCU_EICR1 |= (0x1 << FEN0)      |                       // Falling edge enable
                 (0x1 << FEN1)      ;

    SCU_EICR1 |= (0x1 << EIEN0)     |                       // The trigger event is enabled
                 (0x1 << EIEN1)     ;

    SCU_EICR1 |= (0x0 << INP0)      |                       // Trigger Input Channel 2 -> Output Channel 0
                 (0x1 << INP1)      ;                       // Trigger Input Channel 3 -> Output Channel 1

    SCU_IGCR0 |= (0x1 << IGP0)      |                       // Input Channel 2 activated, pattern is not considered
                 (0x1 << IGP1)      ;                       // Input Channel 3 activated, pattern is not considered

    /* Password Access to unlock WDTSCON0 */
    SCU_WDTSCON0 = ((SCU_WDTSCON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDTSCON0 & (1 << LCK)) != 0);

    /* Modify Access to set ENDINIT bit */
    SCU_WDTSCON0 = ((SCU_WDTSCON0 ^ 0xFC) | (1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDTSCON0 & (1 << LCK)) == 0);

    /* SRC Interrupt Setting For ECU */
    SRC_SCUERU0 &= ~((0xFF) << SRPN);           // Set Priority : 0x0A
    SRC_SCUERU0 |= ((0x0A) << SRPN);

    SRC_SCUERU0 |= (1 << SRE);                  // Service Request is enabled

    SRC_SCUERU1 &= ~((0xFF) << SRPN);           // Set Priority : 0x0B
    SRC_SCUERU1 |= ((0x0B) << SRPN);

    SRC_SCUERU1 &= ~((0x3) << TOS);             // CPU0 services

    SRC_SCUERU1 |= (1 << SRE);                  // Service Request is enabled
}

volatile int sw1_cnt;
volatile int sw2_cnt;
volatile unsigned int systick_curr;
volatile unsigned int systick_prev;
volatile unsigned int systick;

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

    sw1_cnt = 0;
    sw2_cnt = 0;
    
    init_ERU();                                // Initialize ERU
    init_LED();                                // Initialize LED
    init_Switch();                             // Initialize Switch

    systick_prev = SYSTEM_TIMER_0_31_0;

    while(1)
    {
        systick_curr = SYSTEM_TIMER_0_31_0;
        systick = systick_curr - systck_prev;

        if( systick > 1000000 )            // 1M/100M = 10ms
        {
            systick_prev = systick_curr;
            if( sw1_cnt != 0 ) sw1_cnt--;
            if( sw2_cnt != 0 ) sw2_cnt--;
        }


    }
    return (1);
}

__interrupt(0x0A) __vector_table(0)
void ERU0_ISR(void)
{
    if( sw2_cnt == 0 )
    {
        PORT10_OMR |= ((1<<PCL1) | (1<<PS1));           // Toggle LED RED
        sw2_cnt = 100;
    }
}

__interrupt(0x0B) __vector_table(0)
void ERU1_ISR(void)
{
    if( sw1_cnt == 0 )
    {
        PORT10_OMR |= ((1<<PCL2) | (1<<PS2));           // Toggle LED BLUE
        sw1_cnt = 100;
    }
}
