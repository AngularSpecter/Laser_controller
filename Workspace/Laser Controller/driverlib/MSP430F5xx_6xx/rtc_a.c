/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//*****************************************************************************
//
//rtc_a.c - Driver for the RTC Module.
//
//*****************************************************************************
#include "inc/hw_regaccess.h"
#include "rtc_a.h"
#ifdef  __IAR_SYSTEMS_ICC__
#include "deprecated/IAR/msp430xgeneric.h"
#else
#include "deprecated/CCS/msp430xgeneric.h"
#endif
#include "assert.h"

//*****************************************************************************
//
//! Starts the RTC.
//!
//! \param baseAddress is the base address of the RTC module.
//!
//! This function clears the RTC main hold bit to allow the RTC to function.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_startClock (uint32_t baseAddress)
{
    HWREG8(baseAddress + OFS_RTCCTL01_H) &= ~(RTCHOLD_H);
}

//*****************************************************************************
//
//! Holds the RTC.
//!
//! \param baseAddress is the base address of the RTC module.
//!
//! This function sets the RTC main hold bit to disable RTC functionality.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_holdClock (uint32_t baseAddress)
{
    HWREG8(baseAddress + OFS_RTCCTL01_H) |= RTCHOLD_H;
}

//*****************************************************************************
//
//! Allows and Sets the frequency output to RTCCLK pin for calibration 
//! measurement.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param frequencySelect is the frequency output to RTCCLK.
//!        Valid values are
//!        \b RTC_A_CALIBRATIONFREQ_OFF - turn off calibration output [Default]
//!        \b RTC_A_CALIBRATIONFREQ_512HZ - output signal at 512Hz for calibration
//!        \b RTC_A_CALIBRATIONFREQ_256HZ - output signal at 256Hz for calibration
//!        \b RTC_A_CALIBRATIONFREQ_1HZ - output signal at 1Hz for calibration
//!        Modified bits are \b RTCCALF of \b RTCCTL3 register.
//!
//! This function sets a frequency to measure at the RTCCLK output pin. After
//! testing the set freqeuncy, the calibration could be set accordingly.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setCalibrationFrequency (uint32_t baseAddress,
    uint16_t frequencySelect)
{
    HWREG8(baseAddress + OFS_RTCCTL23) &= ~(RTCCALF_3);
    HWREG8(baseAddress + OFS_RTCCTL23) |= frequencySelect;
}

//*****************************************************************************
//
//! Sets the specified calibration for the RTC.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param offsetDirection is the direction that the calibration offset will go.
//!        Valid values are
//!        \b RTC_A_CALIBRATION_DOWN2PPM - calibrate at steps of -2
//!        \b RTC_A_CALIBRATION_UP4PPM - calibrat at steps of +4
//!        Modified bits are \b RTCCALS of \b RTCCTL2 register.
//! \param offsetValue is the value that the offset will be a factor of; a valid
//!       value is any integer from 1-63.
//!        Modified bits are \b RTCCAL of \b RTCCTL2 register.
//!
//! This function sets the calibration offset to make the RTC as accurate as
//! possible. The offsetDirection can be either +4-ppm or -2-ppm, and the
//! offsetValue should be from 1-63 and is multiplied by the direction setting
//! (i.e. +4-ppm * 8 (offsetValue) = +32-ppm). Please note, when measuring the
//! frequency after setting the calibration, you will only see a change on the
//! 1Hz frequency.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setCalibrationData (uint32_t baseAddress,
    uint8_t offsetDirection,
    uint8_t offsetValue)
{
    HWREG8(baseAddress + OFS_RTCCTL23_L) = offsetValue + offsetDirection;
}

//*****************************************************************************
//
//! Initializes the settings to operate the RTC in Counter mode (RTC_A_A ONLY).
//!
//! \param baseAddress is the base address of the RTC module.
//! \param clockSelect is the selected clock for the counter mode to use.
//!        Valid values are
//!        \b RTC_A_CLOCKSELECT_ACLK [Default]
//!        \b RTC_A_CLOCKsELECT_SMCLK
//!        \b RTC_A_CLOCKSELECT_RT1PS - use Prescaler 1 as source to RTC
//!        Modified bits are \b RTCSSEL of \b RTCCTL1 register.
//! \param counterSizeSelect is the size of the counter.
//!        Valid values are
//!        \b RTC_A_COUNTERSIZE_8BIT [Default]
//!        \b RTC_A_COUNTERSIZE_16BIT
//!        \b RTC_A_COUNTERSIZE_24BIT
//!        \b RTC_A_COUNTERSIZE_32BIT
//!        Modified bits are \b RTCTEV of \b RTCCTL1 register.
//!
//! This function initializes the Counter mode of the RTC_A_A. Setting the clock
//! source and counter size will allow an interrupt from the RTCTEVIFG once an
//! overflow to the counter register occurs.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_counterInit (uint32_t baseAddress,
    uint16_t clockSelect,
    uint16_t counterSizeSelect)
{
    HWREG8(baseAddress + OFS_RTCCTL01_H) |= RTCHOLD;
    HWREG8(baseAddress + OFS_RTCCTL01_H) &= ~(RTCMODE_H);

    HWREG16(baseAddress + OFS_RTCCTL01) &= 0xF0FF; //~(RTCSSEL_3 + RTCTEV_3);
    HWREG16(baseAddress + OFS_RTCCTL01) |= clockSelect + counterSizeSelect;
}

//*****************************************************************************
//
//! Initializes the settings to operate the RTC in Calendar mode.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param CalendarTime is the structure containing the values for the Calendar
//!       to be intiialized to.
//!        Valid values should be of type Calendar and should contain the
//!        following members and corresponding values:
//!        \b Seconds between 0-59
//!        \b Minutes between 0-59
//!        \b Hours between 0-24
//!        \b DayOfWeek between 0-6
//!        \b DayOfMonth between 0-31
//!        \b Year between 0-4095
//!        NOTE: Values beyond the ones specified may result in eradic behavior.
//! \param formatSelect is the format for the Calendar registers to use.
//!        Valid values are
//!        \b RTC_A_FORMAT_BINARY [Default]
//!        \b RTC_A_FORMAT_BCD
//!        Modified bits are \b RTCBCD of \b RTCCTL1 register.
//!
//! This function initializes the Calendar mode of the RTC module. In an RTC_A_A
//! module this mode automatically sets the clock source to ACLK and initializes
//! the prescaler sources and dividers.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_calendarInit (uint32_t baseAddress,
    Calendar CalendarTime,
    uint16_t formatSelect)
{
    HWREG8(baseAddress + OFS_RTCCTL01_H) |= RTCMODE_H + RTCHOLD;

    HWREG16(baseAddress + OFS_RTCCTL01) &= ~(RTCBCD);
    HWREG16(baseAddress + OFS_RTCCTL01) |= formatSelect;

    HWREG8(baseAddress + OFS_RTCTIM0_L) = CalendarTime.Seconds;
    HWREG8(baseAddress + OFS_RTCTIM0_H) = CalendarTime.Minutes;
    HWREG8(baseAddress + OFS_RTCTIM1_L) = CalendarTime.Hours;
    HWREG8(baseAddress + OFS_RTCTIM1_H) = CalendarTime.DayOfWeek;
    HWREG8(baseAddress + OFS_RTCDATE_L) = CalendarTime.DayOfMonth;
    HWREG8(baseAddress + OFS_RTCDATE_H) = CalendarTime.Month;
    HWREG16(baseAddress + OFS_RTCYEAR) = CalendarTime.Year;
}

//*****************************************************************************
//
//! Returns the Calendar Time stored in the Calendar registers of the RTC.
//!
//! \param baseAddress is the base address of the RTC module.
//!
//! This function returns the current Calendar time in the form of a Calendar
//! structure.
//!
//! \return A Calendar structure containing the current time.
//
//*****************************************************************************
Calendar RTC_A_getCalendarTime (uint32_t baseAddress)
{
    Calendar tempCal;

    while ( !(HWREG16(baseAddress + OFS_RTCCTL01) & RTCRDY) ) ;

    tempCal.Seconds    = HWREG8(baseAddress + OFS_RTCTIM0_L);
    tempCal.Minutes    = HWREG8(baseAddress + OFS_RTCTIM0_H);
    tempCal.Hours      = HWREG8(baseAddress + OFS_RTCTIM1_L);
    tempCal.DayOfWeek  = HWREG8(baseAddress + OFS_RTCTIM1_H);
    tempCal.DayOfMonth = HWREG8(baseAddress + OFS_RTCDATE_L);
    tempCal.Month      = HWREG8(baseAddress + OFS_RTCDATE_H);
    tempCal.Year       = HWREG16(baseAddress + OFS_RTCYEAR);

    return ( tempCal) ;
}

//*****************************************************************************
//
//! Sets and Enables the desired Calendar Alarm settings.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param minutesAlarm is the alarm condition for the minutes.
//!        Valid values are
//!        \b An integer between 0-59, OR
//!        \b RTC_A_ALARMCONDITION_OFF [Default]
//! \param hoursAlarm is the alarm condition for the hours.
//!        Valid values are
//!        \b An integer between 0-24, OR
//!        \b RTC_A_ALARMCONDITION_OFF [Default]
//! \param dayOfWeekAlarm is the alarm condition for the day of week.
//!        Valid values are
//!        \b An integer between 0-6, OR
//!        \b RTC_A_ALARMCONDITION_OFF [Default]
//! \param dayOfMonthAlarm is the alarm condition for the day of the month.
//!        Valid values are
//!        \b An integer between 0-31, OR
//!        \b RTC_A_ALARMCONDITION_OFF [Default]
//!
//! This function sets a Calendar interrupt condition to assert the RTCAIFG
//! interrupt flag. The condition is a logical and of all of the parameters.
//! For example if the minutes and hours alarm is set, then the interrupt will
//! only assert when the minutes AND the hours change to the specified setting.
//! Use the RTC_A_ALARM_OFF for any alarm settings that should not be apart of the
//! alarm condition.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setCalendarAlarm (uint32_t baseAddress,
    uint8_t minutesAlarm,
    uint8_t hoursAlarm,
    uint8_t dayOfWeekAlarm,
    uint8_t dayOfMonthAlarm)
{
    //Each of these is XORed with 0x80 to turn on if an integer is passed,
    //or turn OFF if RTC_A_ALARM_OFF (0x80) is passed.
    HWREG8(baseAddress + OFS_RTCAMINHR_L) = (minutesAlarm ^ 0x80);
    HWREG8(baseAddress + OFS_RTCAMINHR_H) = (hoursAlarm ^ 0x80);
    HWREG8(baseAddress + OFS_RTCADOWDAY_L) = (dayOfWeekAlarm ^ 0x80);
    HWREG8(baseAddress + OFS_RTCADOWDAY_H) = (dayOfMonthAlarm ^ 0x80);
}

//*****************************************************************************
//
//! Sets a single specified Calendar interrupt condition.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param eventSelect is the condition selected.
//!        Valid values are
//!        \b RTC_A_CALENDAREVENT_MINUTECHANGE - assert interrupt on every minute
//!        \b RTC_A_CALENDAREVENT_HOURCHANGE - assert interrupt on every hour
//!        \b RTC_A_CALENDAREVENT_NOON - assert interrupt when hour is 12
//!        \b RTC_A_CALENDAREVENT_MIDNIGHT - assert interrupt when hour is 0
//!        Modified bits are \b RTCTEV of \b RTCCTL register.
//!
//! This function sets a specified event to assert the RTCTEVIFG interrupt. This
//! interrupt is independent from the Calendar alarm interrupt.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setCalendarEvent (uint32_t baseAddress,
    uint16_t eventSelect)
{
    HWREG16(baseAddress + OFS_RTCCTL01) &= ~(RTCTEV_3); //Reset bits
    HWREG16(baseAddress + OFS_RTCCTL01) |= eventSelect;
}

//*****************************************************************************
//
//! Returns the value of the Counter register (RTC_A_A ONLY).
//!
//! \param baseAddress is the base address of the RTC module.
//!
//! This function returns the value of the counter register for the RTC_A_A module.
//! It will return the 32-bit value no matter the size set during initialization.
//! The RTC should be held before trying to use this function.
//!
//! \return The raw value of the full 32-bit Counter Register.
//
//*****************************************************************************
uint32_t RTC_A_getCounterValue (uint32_t baseAddress)
{
    if ( (HWREG8(baseAddress + OFS_RTCCTL01_H) & RTCHOLD_H)
         || (HWREG8(baseAddress + OFS_RTCPS1CTL) & RT1PSHOLD) ){
        return ( 0) ;
    }

    uint32_t counterValue_L = HWREG16(baseAddress + OFS_RTCTIM0);
    uint32_t counterValue_H = HWREG16(baseAddress + OFS_RTCTIM1);
    return ( (counterValue_H << 16) + counterValue_L );
}

//*****************************************************************************
//
//! Sets the value of the Counter register 
//!
//! \param baseAddress is the base address of the RTC module.
//! \param counterValue is the value to set the Counter register to; a valid
//!       value may be any 32-bit integer.
//!
//! This function sets the counter register of the RTC_A_A module.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setCounterValue (uint32_t baseAddress,
    uint32_t counterValue)
{
    HWREG16(baseAddress + OFS_RTCTIM0) = counterValue;
    HWREG16(baseAddress + OFS_RTCTIM1) = ( counterValue >> 16 );
}

//*****************************************************************************
//
//! Initializes the Prescaler for Counter mode (RTC_A_A ONLY).
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to initialize.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//! \param prescaleClockSelect is the clock to drive the selected prescaler.
//!        Valid values are
//!        \b RTC_A_CLOCKSELECT_ACLK [Default]
//!        \b RTC_A_CLOCKSELECT_SMCLK
//!        \b RTC_A_CLOCKSELECT_RT0PS - use Prescaler 0 as source to Prescaler 1
//!             (May only be used if prescaleSelect is RTC_A_PRESCALE_1)
//!        Modified bits are \b RTxSSEL of \b RTCPSxCTL register.
//! \param prescaleDivider is the divider for the selected clock source.
//!        Valid values are
//!        \b RTC_A_PSDIVIDER_2 [Default]
//!        \b RTC_A_PSDIVIDER_4
//!        \b RTC_A_PSDIVIDER_8
//!        \b RTC_A_PSDIVIDER_16
//!        \b RTC_A_PSDIVIDER_32
//!        \b RTC_A_PSDIVIDER_64
//!        \b RTC_A_PSDIVIDER_128
//!        \b RTC_A_PSDIVIDER_256
//!        Modified bits are \b RTxPSDIV of \b RTCPSxCTL register.
//!
//! This function initializes the selected prescaler for the counter mode in the
//! RTC_A_A module. If the RTC is initialized in Calendar mode, then these are
//! automatically initialized. The Prescalers can be used to divide a clock
//! source additionally before it gets to the main RTC clock.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_counterPrescaleInit (uint32_t baseAddress,
    uint8_t prescaleSelect,
    uint16_t prescaleClockSelect,
    uint16_t prescaleDivider)
{
    //Reset bits and set clock select
    HWREG16(baseAddress + OFS_RTCPS0CTL + prescaleSelect) =
        prescaleClockSelect + prescaleDivider;
}

//*****************************************************************************
//
//! Holds the selected Prescaler (RTC_A_A ONLY).
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to hold.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//!
//! This function holds the prescale counter from continuing. This will only
//! work in counter mode, in Calendar mode, the RTC_A_holdClock() must be used.
//! In counter mode, if using both prescalers in conjunction with the main RTC
//! counter, then stopping RT0PS will stop RT1PS, but stopping RT1PS will not
//! stop RT0PS.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_counterPrescaleHold (uint32_t baseAddress,
    uint8_t prescaleSelect)
{
    HWREG8(baseAddress + OFS_RTCPS0CTL_H + prescaleSelect) |= RT0PSHOLD_H;
}

//*****************************************************************************
//
//! Starts the selected Prescaler (RTC_A_A ONLY).
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to start.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//!
//! This function starts the selected prescale counter. This function will only
//! work if the RTC is in counter mode.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_counterPrescaleStart (uint32_t baseAddress,
    uint8_t prescaleSelect)
{
    HWREG8(baseAddress + OFS_RTCPS0CTL_H + prescaleSelect) &= ~(RT0PSHOLD_H);
}

//*****************************************************************************
//
//! Sets up an interrupt condition for the selected Prescaler.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to define an interrupt for.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//! \param prescaleEventDivider is a divider to specify when an interrupt can
//!       occur based on the clock source of the selected prescaler.
//!       (Does not affect timer of the selected prescaler).
//!       Valid values are
//!       \b RTC_A_PSEVENTDIVIDER_2 [Default]
//!       \b RTC_A_PSEVENTDIVIDER_4
//!       \b RTC_A_PSEVENTDIVIDER_8
//!       \b RTC_A_PSEVENTDIVIDER_16
//!       \b RTC_A_PSEVENTDIVIDER_32
//!       \b RTC_A_PSEVENTDIVIDER_64
//!       \b RTC_A_PSEVENTDIVIDER_128
//!       \b RTC_A_PSEVENTDIVIDER_256
//!        Modified bits are \b RTxIP of \b RTCPSxCTL register.
//!
//! This function sets the condition for an interrupt to assert based on the
//! individual prescalers.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_definePrescaleEvent (uint32_t baseAddress,
    uint8_t prescaleSelect,
    uint8_t prescaleEventDivider)
{
    HWREG8(baseAddress + OFS_RTCPS0CTL_L + prescaleSelect) &= ~(RT0IP_7);
    HWREG8(baseAddress + OFS_RTCPS0CTL_L +
        prescaleSelect) |= prescaleEventDivider;
}

//*****************************************************************************
//
//! Returns the selected Prescaler value.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to obtain the value of.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//!
//! This function returns the value of the selected prescale counter register.
//! The counter should be held before reading. If in counter mode, the individual
//! prescaler can be held, while in Calendar mode the whole RTC must be held.
//!
//! \return The value of the specified Prescaler count register
//
//*****************************************************************************
uint8_t RTC_A_getPrescaleValue (uint32_t baseAddress,
    uint8_t prescaleSelect)
{
    if (HWREG8(baseAddress + OFS_RTCPS0CTL_H + prescaleSelect) & RT0PSHOLD_H){
        return ( 0) ;
    }

    if (RTC_A_PRESCALE_0 == prescaleSelect){
        return ( HWREG8(baseAddress + OFS_RTCPS_L) );
    } else if (RTC_A_PRESCALE_1 == prescaleSelect){
        return ( HWREG8(baseAddress + OFS_RTCPS_H) );
    } else   {
        return ( 0) ;
    }
}

//*****************************************************************************
//
//! Sets the selected Prescaler value.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param prescaleSelect is the prescaler to set the value for.
//!        Valid values are
//!        \b RTC_A_PRESCALE_0
//!        \b RTC_A_PRESCALE_1
//! \param prescaleCounterValue is the specified value to set the prescaler to;
//!       a valid value is any integer from 0-255.
//!        Modified bits are \b RTxPS of \b RTxPS register.
//!
//! This function sets the prescale counter value. Before setting the prescale
//! counter, it should be held.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_setPrescaleCounterValue (uint32_t baseAddress,
    uint8_t prescaleSelect,
    uint8_t prescaleCounterValue)
{
    if (RTC_A_PRESCALE_0 == prescaleSelect){
        HWREG8(baseAddress + OFS_RTCPS_L) = prescaleCounterValue;
    } else if (RTC_A_PRESCALE_1 == prescaleSelect){
        HWREG8(baseAddress + OFS_RTCPS_H) = prescaleCounterValue;
    }
}

//*****************************************************************************
//
//! Enables selected RTC interrupt sources.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param interruptMask is a bit mask of the interrupts to enable.
//!        Mask Value is the logical OR of any of the following
//!        \b RTC_A_TIME_EVENT_INTERRUPT - asserts when counter overflows in 
//!             counter mode or when Calendar event condition defined by 
//!             defineCalendarEvent() is met.
//!        \b RTC_A_CLOCK_ALARM_INTERRUPT - asserts when alarm condition in 
//!             Calendar mode is met.
//!        \b RTC_A_CLOCK_READ_READY_INTERRUPT - asserts when Calendar registers 
//!             are settled.
//!        \b RTC_A_PRESCALE_TIMER0_INTERRUPT - asserts when Prescaler 0 event 
//!             condition is met.
//!        \b RTC_A_PRESCALE_TIMER1_INTERRUPT - asserts when Prescaler 1 event 
//!             condition is met.
//!
//! This function enables the selected RTC interrupt source.  Only the sources
//! that are enabled can be reflected to the processor interrupt; disabled
//! sources have no effect on the processor. <b>Does not clear interrupt 
//! flags.</b>
//!
//! \return None
//
//*****************************************************************************
void RTC_A_enableInterrupt (uint32_t baseAddress,
    uint8_t interruptMask)
{
    if ( interruptMask & (RTCTEVIE + RTCAIE + RTCRDYIE) ){
        HWREG8(baseAddress + OFS_RTCCTL01_L) |=
            (interruptMask & (RTCTEVIE + RTCAIE + RTCRDYIE));
    }

    if (interruptMask & RTC_A_PRESCALE_TIMER0_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS0CTL) |= RT0PSIE;
    }

    if (interruptMask & RTC_A_PRESCALE_TIMER1_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS1CTL) |= RT1PSIE;
    }
}

//*****************************************************************************
//
//! Disables selected RTC interrupt sources.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param interruptMask is a bit mask of the interrupts to disable.
//!        Mask Value is the logical OR of any of the following
//!        \b RTC_A_TIME_EVENT_INTERRUPT - asserts when counter overflows in 
//!             counter mode or when Calendar event condition defined by 
//!             defineCalendarEvent() is met.
//!        \b RTC_A_CLOCK_ALARM_INTERRUPT - asserts when alarm condition in 
//!             Calendar mode is met.
//!        \b RTC_A_CLOCK_READ_READY_INTERRUPT - asserts when Calendar registers 
//!             are settled.
//!        \b RTC_A_PRESCALE_TIMER0_INTERRUPT - asserts when Prescaler 0 event 
//!             condition is met.
//!        \b RTC_A_PRESCALE_TIMER1_INTERRUPT - asserts when Prescaler 1 event 
//!             condition is met.
//!
//! This function disables the selected RTC interrupt source.  Only the sources
//! that are enabled can be reflected to the processor interrupt; disabled
//! sources have no effect on the processor.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_disableInterrupt (uint32_t baseAddress,
    uint8_t interruptMask)
{
    if ( interruptMask & ( RTCTEVIE + RTCAIE + RTCRDYIE) ){
        HWREG8(baseAddress + OFS_RTCCTL01_L) &=
            ~(interruptMask & (RTCTEVIE + RTCAIE + RTCRDYIE));
    }

    if (interruptMask & RTC_A_PRESCALE_TIMER0_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS0CTL) &= ~(RT0PSIE);
    }

    if (interruptMask & RTC_A_PRESCALE_TIMER1_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS1CTL) &= ~(RT1PSIE);
    }
}

//*****************************************************************************
//
//! Returns the status of the selected interrupts flags.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param interruptFlagMask is a bit mask of the interrupt flags to return the
//!       status of.
//!        Mask Value is the logical OR of any of the following
//!        \b RTC_A_TIME_EVENT_INTERRUPT - asserts when counter overflows in 
//!             counter mode or when Calendar event condition defined by 
//!             defineCalendarEvent() is met.
//!        \b RTC_A_CLOCK_ALARM_INTERRUPT - asserts when alarm condition in 
//!             Calendar mode is met.
//!        \b RTC_A_CLOCK_READ_READY_INTERRUPT - asserts when Calendar registers 
//!             are settled.
//!        \b RTC_A_PRESCALE_TIMER0_INTERRUPT - asserts when Prescaler 0 event 
//!             condition is met.
//!        \b RTC_A_PRESCALE_TIMER1_INTERRUPT - asserts when Prescaler 1 event 
//!             condition is met.
//!
//! This function returns the status of the interrupt flag for the selected
//! channel.
//!
//! \return A bit mask of the selected interrupt flag's status.
//
//*****************************************************************************
uint8_t RTC_A_getInterruptStatus (uint32_t baseAddress,
    uint8_t interruptFlagMask)
{
    uint8_t tempInterruptFlagMask = 0x0000;

    tempInterruptFlagMask |= (HWREG8(baseAddress + OFS_RTCCTL01_L)
                              & ((interruptFlagMask >> 4)
                                 & (
                                    RTCTEVIFG + 
                                    RTCAIFG + 
                                    RTCRDYIFG)));
    
    tempInterruptFlagMask = tempInterruptFlagMask << 4;

    if (interruptFlagMask & RTC_A_PRESCALE_TIMER0_INTERRUPT){
        if ( HWREG8(baseAddress + OFS_RTCPS0CTL) & RT0PSIFG){
            tempInterruptFlagMask |= RTC_A_PRESCALE_TIMER0_INTERRUPT;
        }
    }

    if (interruptFlagMask & RTC_A_PRESCALE_TIMER1_INTERRUPT){
        if ( HWREG8(baseAddress + OFS_RTCPS1CTL) & RT1PSIFG){
            tempInterruptFlagMask |= RTC_A_PRESCALE_TIMER1_INTERRUPT;
        }
    }

    return ( tempInterruptFlagMask) ;
}

//*****************************************************************************
//
//! Clears selected RTC interrupt flags.
//!
//! \param baseAddress is the base address of the RTC module.
//! \param interruptFlagMask is a bit mask of the interrupt flags to be cleared.
//!        Mask Value is the logical OR of any of the following
//!        \b RTC_A_TIME_EVENT_INTERRUPT - asserts when counter overflows in 
//!             counter mode or when Calendar event condition defined by 
//!             defineCalendarEvent() is met.
//!        \b RTC_A_CLOCK_ALARM_INTERRUPT - asserts when alarm condition in 
//!             Calendar mode is met.
//!        \b RTC_A_CLOCK_READ_READY_INTERRUPT - asserts when Calendar registers 
//!             are settled.
//!        \b RTC_A_PRESCALE_TIMER0_INTERRUPT - asserts when Prescaler 0 event 
//!             condition is met.
//!        \b RTC_A_PRESCALE_TIMER1_INTERRUPT - asserts when Prescaler 1 event 
//!             condition is met.
//!
//! This function clears the RTC interrupt flag is cleared, so that it no longer
//! asserts.
//!
//! \return None
//
//*****************************************************************************
void RTC_A_clearInterrupt (uint32_t baseAddress,
    uint8_t interruptFlagMask)
{
    if ( interruptFlagMask & (RTC_A_TIME_EVENT_INTERRUPT + 
                              RTC_A_CLOCK_ALARM_INTERRUPT + 
                              RTC_A_CLOCK_READ_READY_INTERRUPT  
                              ) ){
      
        HWREG8(baseAddress + OFS_RTCCTL01_L) &=
            ~((interruptFlagMask>>4) & (RTCTEVIFG +
                                        RTCAIFG + 
                                        RTCRDYIFG));
    }

    if (interruptFlagMask & RTC_A_PRESCALE_TIMER0_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS0CTL) &= ~(RT0PSIFG);
    }

    if (interruptFlagMask & RTC_A_PRESCALE_TIMER1_INTERRUPT){
        HWREG8(baseAddress + OFS_RTCPS1CTL) &= ~(RT1PSIFG);
    }
}


//*****************************************************************************
//
//Close the Doxygen group.
//! @}
//
//*****************************************************************************
