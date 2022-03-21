#include "my_lib.h"

void init_VADC(void)
{
    /* VADC Enable */
    /* Password Access to unlock WDTCPU0CON0 */
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) != 0);

    /* Modify Access to clear ENDINIT bit */
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) | (1 << LCK)) & ~ (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) == 0);

    VADC_CLC &= ~(1 << DISR);                 // Enable VADC Module

    /* Password Access to unlock WDTSCPU0CON0 */
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) & ~(1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) != 0);

    /* Modify Access to clear ENDINIT bit */
    SCU_WDT_CPU0CON0 = ((SCU_WDT_CPU0CON0 ^ 0xFC) | (1 << LCK)) | (1 << ENDINIT);
    while((SCU_WDT_CPU0CON0 & (1 << LCK)) == 0);

    while((VADC_CLC & (1 << DISS)) != 0);     // Wait until module is enabled

    VADC_G4ARBPR |= ((0x3) << PRIO0);         // Highest Priority for Request Source 0
    VADC_G4ARBPR &= ~(1 << CSM0);             // Conversion Start Mode : Wait-for-start mode
    VADC_G4ARBPR |= (1 << ASEN0);             // Arbitration Source Input 0 Enable

    VADC_G4QMR0  &= ~((0x3) << ENGT);         // Enable Conversion Requests
    VADC_G4QMR0  |= ((0x1) << ENGT);

    VADC_G4QMR0  |= (1 << FLUSH);             // Clear all Queue Entries

    VADC_G4ARBCFG |= ((0x3) << ANONC);        // Analog Converter : Normal Operation

    VADC_G4ICLASS0 &= ~((0x7) << CMS);        // Group-specific Class 0
                                              // Conversion Mode : Standard Conversion (12-bit)

    /* VADC Group 4 Channel 7 Setting */
    VADC_G4CHCTR7 |= (1 << RESPOS);           // Read Results Right-aligned
    VADC_G4CHCTR7 &= ~((0xF) << RESREG);      // Store Result in Group Result Register G4RES1
    VADC_G4CHCTR7 |= (1 << RESREG);
    VADC_G4CHCTR7 &= ~((0x3) << ICLSEL);      // Use Group-specific Class 0

    /* VADC Group 4 Channel 6 Setting */
    VADC_G4CHCTR6 |= (1 << RESPOS);           // Read Results Right-aligned
    VADC_G4CHCTR6 &= ~((0xF) << RESREG);      // Store Result in Group Result Register G4RES1
    VADC_G4CHCTR6 |= (1 << RESREG);
    VADC_G4CHCTR6 &= ~((0x3) << ICLSEL);      // Use Group-specific Class 0
}


unsigned int GetVADC4(int channel)
{
    unsigned int result;

    result = 0;
    for( int i=0; i<16; i++)
    {
        VADC_G4QINR0 = channel;
        VADC_G4QMR0 |= (1 << TREV);                     // Generate a Trigger Event
        while((VADC_G4RES1 & (1 << VF)) == 0);          // Wait until New Result Available
        result += (VADC_G4RES1 & ((0xFFFF) << RESULT));  // Read Result
    }
    return result>>4;
}

