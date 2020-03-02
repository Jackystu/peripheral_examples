/**************************************************************************//**
 * @main.c
 * @brief This project demonstrates the ability for a pin to wake the device
 * from EM4.
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>Copyright 2020 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_rmu.h"
#include "mx25flash_spi.h"
#include "bsp.h"

#define EM4WU_PIN           BSP_GPIO_PB1_PIN
#define EM4WU_PORT          BSP_GPIO_PB1_PORT
#define EM4WU_EM4WUEN_NUM   (3)                       // PB1 is EM4WUEN pin 3
#define EM4WU_EM4WUEN_MASK  (1 << EM4WU_EM4WUEN_NUM)

/**************************************************************************//**
 * A JEDEC standard SPI flash boots up in standby mode in order to
 * provide immediate access, such as when used it as a boot memory.
 *
 * Typical current draw in standby mode for the MX25R8035F device used
 * on EFR32 radio boards is 5 �A.
 *
 * JEDEC standard SPI flash memories have a lower current deep power-down mode,
 * which can be entered after sending the relevant commands.  This is on the
 * order of 0.007 �A for the MX25R8035F.
 *****************************************************************************/
void powerDownSpiFlash(void)
{
  FlashStatus status;

  MX25_init();
  MX25_RSTEN();
  MX25_RST(&status);
  MX25_DP();
  MX25_deinit();
}

/**************************************************************************//**
 * @brief  Initialize GPIOs for push button and LED
 *****************************************************************************/
void initGPIO(void)
{
  // Configure GPIO pins
  CMU_ClockEnable(cmuClock_GPIO, true);
  // Configure Button PB0 as input for the escape hatch
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT,BSP_GPIO_PB0_PIN, gpioModeInput, 1);
  // Configure Button PB1 as input and EM4 wake-up source
  GPIO_PinModeSet(EM4WU_PORT, EM4WU_PIN, gpioModeInputPullFilter, 1);
  // Enable GPIO pin wake-up from EM4
  GPIO_EM4EnablePinWakeup(EM4WU_EM4WUEN_MASK << _GPIO_EM4WUEN_EM4WUEN_SHIFT, 0);

  // Configure LED0 as output
  GPIO_PinModeSet(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN, gpioModePushPull, 0);
}

/**************************************************************************//**
 * @brief Toggle LEDs on STK and Radio board indefinitely
 *****************************************************************************/
void toggleLEDs(void)
{
  while(1)
  {
    GPIO_PinOutToggle(BSP_GPIO_LED0_PORT, BSP_GPIO_LED0_PIN);
    // Arbitrary delay between toggles
    for(volatile uint32_t delay = 0; delay < 0xFFFFF; delay++);
  }
}

/**************************************************************************//**
 * @brief Main function
 *****************************************************************************/
int main(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;

  CHIP_Init();

  // Turn on DCDC regulator
  EMU_DCDCInit(&dcdcInit);

  // Release pin state
  EMU_UnlatchPinRetention();

  //Initialization
  initGPIO();
  /**********************************************************************//**
   * When developing/debugging code on xG22 that enters EM2 or lower,
   * it's a good idea to have an "escape hatch" type mechanism, e.g. a
   * way to pause the device so that a debugger can connect in order
   * to erase flash, among other things.
   *
   * Before proceeding with this example, make sure PB0 is not pressed.
   * If the PB0 pin is low, turn on LED1 and execute the breakpoint
   * instruction to stop the processor in EM0 and allow a debug
   * connection to be made.
   *************************************************************************/
  if (GPIO_PinInGet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN) == 0)
  {
    GPIO_PinOutSet(BSP_GPIO_LED1_PORT, BSP_GPIO_LED1_PIN);
    __BKPT(0);
  }
  // Pin not asserted, so disable input
  else
  {
    GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeDisabled, 0);
  }

  // Get the last Reset Cause
  uint32_t rstCause = RMU_ResetCauseGet();
  // Clear Reset Cause
  RMU_ResetCauseClear();

  // If the last Reset was due to leaving EM4, toggle LEDs. Else, enter EM4
  if(rstCause & EMU_RSTCAUSE_EM4)
  {
    toggleLEDs();
  }
  else
  {
    // Power-down the radio board SPI flash
    powerDownSpiFlash();

    // Switch from DCDC regulation mode to bypass mode.
    EMU_DCDCModeSet(emuDcdcMode_Bypass);
    // Use default settings for EM4, XO's and DCDC
    EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
    // Enable Pin Retention through EM4 and wakeup
    em4Init.pinRetentionMode = emuPinRetentionLatch;

    // Initialize EM mode 4
    EMU_EM4Init(&em4Init);

    // Enter EM4
    EMU_EnterEM4();
  }

  // This line should never be reached
  while(1);
}
