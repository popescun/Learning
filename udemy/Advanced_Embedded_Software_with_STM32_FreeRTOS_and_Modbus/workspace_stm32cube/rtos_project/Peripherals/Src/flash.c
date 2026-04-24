/*
 * flash.c
 *
 * Contains function definitions for configuring the MCU's internal flash memory.
 */

#include "flash.h"

void flash_config_wait_states(uint8_t hclk) {
  // Reset the Flash Access Control Register to the default value after reset.
  // Doc: 3.8.1 Flash access control register (FLASH_ACR): DCRST ICRST DCEN ICEN PRFTEN LATENCY
  FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |
                  FLASH_ACR_DCEN | FLASH_ACR_ICRST | FLASH_ACR_DCRST);

  FLASH->ACR |= (FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN);

  // Calculate the latency based on the hclk value
  uint32_t latency = (hclk - 1) / 30; // Rounds up to nearest multiple of 30

  // Set the FLASH_ACR_LATENCY based on the calculated latency
  FLASH->ACR |= (latency << FLASH_ACR_LATENCY_Pos);
}
