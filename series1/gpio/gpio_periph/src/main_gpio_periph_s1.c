/*****************************************************************************
 * @file
 * @brief GPIO Demo Application for EFM32GG_STK3700
 * @version 2.00
*******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_chip.h"
#include "bsp.h"


#ifdef BSP_STK_BRD2204A /* GG11 STK LFRCO out on PF2 */
#define SQUARE_WAVE_PORT gpioPortF
#define SQUARE_WAVE_PIN  2
#define SQUARE_WAVE_CLKOUTSEL CMU_CTRL_CLKOUTSEL0_LFRCO
#define SQUARE_WAVE_ROUTE_PEN CMU_ROUTEPEN_CLKOUT0PEN
#define SQUARE_WAVE_LOC CMU_ROUTELOC0_CLKOUT0LOC_LOC4
#else /* All other STKs, LFRCO out on PA0 */
#define SQUARE_WAVE_PORT gpioPortA
#define SQUARE_WAVE_PIN  0
#define SQUARE_WAVE_CLKOUTSEL CMU_CTRL_CLKOUTSEL1_LFRCO
#define SQUARE_WAVE_ROUTE_PEN CMU_ROUTEPEN_CLKOUT1PEN
#define SQUARE_WAVE_LOC CMU_ROUTELOC0_CLKOUT1LOC_LOC0
#endif

/**************************************************************************//**
 * @brief  Main function
 * Main is called from __iar_program_start, see assembly startup file
 *****************************************************************************/
int main(void)
{  
  /* Initialize chip */
  CHIP_Init();

  /* Enable clock for GPIO module */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Set PA0/PF2 as output so it can be
  overriden by the peripheral, in this case the CMU */  
  GPIO_PinModeSet(SQUARE_WAVE_PORT, SQUARE_WAVE_PIN, gpioModePushPull, 0);

  /* Enable Low Frequency RC Oscillator (LFRCO) and 
  wait until it is stable*/
  CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

  /* Select Clock Output 1 as Low Frequency RC(32.768 KHz) */
  CMU->CTRL = CMU->CTRL | SQUARE_WAVE_CLKOUTSEL;

  /* Route the clock output to location 2 and enable it */
  CMU->ROUTEPEN = SQUARE_WAVE_ROUTE_PEN;
  CMU->ROUTELOC0 = SQUARE_WAVE_LOC;

  while(1);
}
