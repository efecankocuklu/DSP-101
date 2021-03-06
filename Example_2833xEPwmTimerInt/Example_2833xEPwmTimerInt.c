// TI File $Revision: /main/2 $
// Checkin $Date: July 30, 2009   18:44:34 $
//###########################################################################
//
// FILE:    Example_2833xEPwmTimerInt.c
//
// TITLE:   DSP2833x ePWM Timer Interrupt example.
//
// ASSUMPTIONS:
//
//    This program requires the DSP2833x header files.
//
//    Other then boot mode configuration, no other hardware configuration
//    is required.
//
//    As supplied, this project is configured for "boot to SARAM"
//    operation.  The 2833x Boot Mode table is shown below.
//    For information on configuring the boot mode of an eZdsp,
//    please refer to the documentation included with the eZdsp,
//
//       $Boot_Table:
//
//         GPIO87   GPIO86     GPIO85   GPIO84
//          XA15     XA14       XA13     XA12
//           PU       PU         PU       PU
//        ==========================================
//            1        1          1        1    Jump to Flash
//            1        1          1        0    SCI-A boot
//            1        1          0        1    SPI-A boot
//            1        1          0        0    I2C-A boot
//            1        0          1        1    eCAN-A boot
//            1        0          1        0    McBSP-A boot
//            1        0          0        1    Jump to XINTF x16
//            1        0          0        0    Jump to XINTF x32
//            0        1          1        1    Jump to OTP
//            0        1          1        0    Parallel GPIO I/O boot
//            0        1          0        1    Parallel XINTF boot
//            0        1          0        0    Jump to SARAM	    <- "boot to SARAM"
//            0        0          1        1    Branch to check boot mode
//            0        0          1        0    Boot to flash, bypass ADC cal
//            0        0          0        1    Boot to SARAM, bypass ADC cal
//            0        0          0        0    Boot to SCI-A, bypass ADC cal
//                                              Boot_Table_End$
//
// DESCRIPTION:
//
//    This example configures the ePWM Timers and increments
//    a counter each time an interrupt is taken.
//
//    As supplied:
//
//    All ePWM's are initalized.  Note that not all devices in the 2833x
//    family have all 6 ePWMs.
//
//    All timers have the same period
//    The timers are started sync'ed
//    An interrupt is taken on a zero event for each ePWM timer
//
//       ePWM1: takes an interrupt every event
//       ePWM2: takes an interrupt every 2nd event
//       ePWM3: takes an interrupt every 3rd event
//       ePWM4-ePWM6: take an interrupt every event
//
//    Thus the Interrupt count for ePWM1, ePWM4-ePWM6 should be equal
//    The interrupt count for ePWM2 should be about half that of ePWM1
//    and the interrupt count for ePWM3 should be about 1/3 that of ePWM1
//
//    Watch Variables:
//       EPwm1TimerIntCount
//       EPwm2TimerIntCount
//       EPwm3TimerIntCount
//       EPwm4TimerIntCount
//       EPwm5TimerIntCount
//       EPwm6TimerIntCount
//
//###########################################################################
// $TI Release: DSP2833x/DSP2823x C/C++ Header Files V1.31 $
// $Release Date: August 4, 2009 $
//###########################################################################


#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

// Configure which ePWM timer interrupts are enabled at the PIE level:
// 1 = enabled,  0 = disabled
#define PWM1_INT_ENABLE  1
#define PWM2_INT_ENABLE  1
#define PWM3_INT_ENABLE  1
#define PWM4_INT_ENABLE  1
#define PWM5_INT_ENABLE  1
#define PWM6_INT_ENABLE  1

// Configure the period for each timer
#define PWM1_TIMER_TBPRD   0x1FFF
#define PWM2_TIMER_TBPRD   0x1FFF
#define PWM3_TIMER_TBPRD   0x1FFF
#define PWM4_TIMER_TBPRD   0x1FFF
#define PWM5_TIMER_TBPRD   0x1FFF
#define PWM6_TIMER_TBPRD   0x1FFF


// Prototype statements for functions found within this file.
interrupt void epwm1_timer_isr(void);
interrupt void epwm2_timer_isr(void);
interrupt void epwm3_timer_isr(void);
interrupt void epwm4_timer_isr(void);
interrupt void epwm5_timer_isr(void);
interrupt void epwm6_timer_isr(void);
void InitEPwmTimer(void);

// Global variables used in this example
Uint32  EPwm1TimerIntCount;
Uint32  EPwm2TimerIntCount;
Uint32  EPwm3TimerIntCount;
Uint32  EPwm4TimerIntCount;
Uint32  EPwm5TimerIntCount;
Uint32  EPwm6TimerIntCount;


void main(void)
{
   int i;

// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP2833x_SysCtrl.c file.
   InitSysCtrl();

// Step 2. Initalize GPIO:
// This example function is found in the DSP2833x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
// InitGpio();  // Skipped for this example


// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
   DINT;

// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
// This function is found in DSP2833x_PieVect.c.
   InitPieVectTable();

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
   EALLOW;  // This is needed to write to EALLOW protected registers
   PieVectTable.EPWM1_INT = &epwm1_timer_isr;
   PieVectTable.EPWM2_INT = &epwm2_timer_isr;
   PieVectTable.EPWM3_INT = &epwm3_timer_isr;
   PieVectTable.EPWM4_INT = &epwm4_timer_isr;
   PieVectTable.EPWM5_INT = &epwm5_timer_isr;
   PieVectTable.EPWM6_INT = &epwm6_timer_isr;
   EDIS;    // This is needed to disable write to EALLOW protected registers

// Step 4. Initialize all the Device Peripherals:
// This function is found in DSP2833x_InitPeripherals.c
// InitPeripherals();  // Not required for this example
   InitEPwmTimer();    // For this example, only initialize the ePWM Timers

// Step 5. User specific code, enable interrupts:

// Initalize counters:
   EPwm1TimerIntCount = 0;
   EPwm2TimerIntCount = 0;
   EPwm3TimerIntCount = 0;
   EPwm4TimerIntCount = 0;
   EPwm5TimerIntCount = 0;
   EPwm6TimerIntCount = 0;

// Enable CPU INT3 which is connected to EPWM1-6 INT:
   IER |= M_INT3;

// Enable EPWM INTn in the PIE: Group 3 interrupt 1-6
   PieCtrlRegs.PIEIER3.bit.INTx1 = PWM1_INT_ENABLE;
   PieCtrlRegs.PIEIER3.bit.INTx2 = PWM2_INT_ENABLE;
   PieCtrlRegs.PIEIER3.bit.INTx3 = PWM3_INT_ENABLE;
   PieCtrlRegs.PIEIER3.bit.INTx4 = PWM4_INT_ENABLE;
   PieCtrlRegs.PIEIER3.bit.INTx5 = PWM5_INT_ENABLE;
   PieCtrlRegs.PIEIER3.bit.INTx6 = PWM6_INT_ENABLE;

// Enable global Interrupts and higher priority real-time debug events:
   EINT;   // Enable Global interrupt INTM
   ERTM;   // Enable Global realtime interrupt DBGM

// Step 6. IDLE loop. Just sit and loop forever (optional):
   for(;;)
   {
       asm("          NOP");
       for(i=1;i<=10;i++)
       {}
   }

}


void InitEPwmTimer()
{

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
   EDIS;

   // Setup Sync
   EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm3Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through
   EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Pass through

   // Allow each timer to be sync'ed

   EPwm1Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
   EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;

   EPwm1Regs.TBPHS.half.TBPHS = 100;
   EPwm2Regs.TBPHS.half.TBPHS = 200;
   EPwm3Regs.TBPHS.half.TBPHS = 300;
   EPwm4Regs.TBPHS.half.TBPHS = 400;
   EPwm5Regs.TBPHS.half.TBPHS = 500;
   EPwm6Regs.TBPHS.half.TBPHS = 600;

   EPwm1Regs.TBPRD = PWM1_TIMER_TBPRD;
   EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;    // Count up
   EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;     // Select INT on Zero event
   EPwm1Regs.ETSEL.bit.INTEN = PWM1_INT_ENABLE;  // Enable INT
   EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;           // Generate INT on 1st event


   EPwm2Regs.TBPRD = PWM2_TIMER_TBPRD;
   EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm2Regs.ETSEL.bit.INTEN = PWM2_INT_ENABLE;   // Enable INT
   EPwm2Regs.ETPS.bit.INTPRD = ET_2ND;            // Generate INT on 2nd event


   EPwm3Regs.TBPRD = PWM3_TIMER_TBPRD;
   EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm3Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm3Regs.ETSEL.bit.INTEN = PWM3_INT_ENABLE;   // Enable INT
   EPwm3Regs.ETPS.bit.INTPRD = ET_3RD;            // Generate INT on 3rd event

   EPwm4Regs.TBPRD = PWM4_TIMER_TBPRD;
   EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm4Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm4Regs.ETSEL.bit.INTEN = PWM4_INT_ENABLE;   // Enable INT
   EPwm4Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event


   EPwm5Regs.TBPRD = PWM5_TIMER_TBPRD;
   EPwm5Regs.TBCTL.bit.CTRMODE= TB_COUNT_UP;      // Count up
   EPwm5Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm5Regs.ETSEL.bit.INTEN = PWM5_INT_ENABLE;   // Enable INT
   EPwm5Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event


   EPwm6Regs.TBPRD = PWM6_TIMER_TBPRD;
   EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;     // Count up
   EPwm6Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;      // Enable INT on Zero event
   EPwm6Regs.ETSEL.bit.INTEN = PWM6_INT_ENABLE;   // Enable INT
   EPwm6Regs.ETPS.bit.INTPRD = ET_1ST;            // Generate INT on 1st event

   EALLOW;
   SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
   EDIS;


}


// Interrupt routines uses in this example:
interrupt void epwm1_timer_isr(void)
{
   EPwm1TimerIntCount++;

   // Clear INT flag for this timer
   EPwm1Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm2_timer_isr(void)
{
   EPwm2TimerIntCount++;

   // Clear INT flag for this timer
   EPwm2Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm3_timer_isr(void)
{
   EPwm3TimerIntCount++;

   // Clear INT flag for this timer
   EPwm3Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm4_timer_isr(void)
{
   EPwm4TimerIntCount++;

   // Clear INT flag for this timer
   EPwm4Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm5_timer_isr(void)
{
   EPwm5TimerIntCount++;

   // Clear INT flag for this timer
   EPwm5Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void epwm6_timer_isr(void)
{
   EPwm6TimerIntCount++;

   // Clear INT flag for this timer
   EPwm6Regs.ETCLR.bit.INT = 1;

   // Acknowledge this interrupt to receive more interrupts from group 3
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}




//===========================================================================
// No more.
//===========================================================================
