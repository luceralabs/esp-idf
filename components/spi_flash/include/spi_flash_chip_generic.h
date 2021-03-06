// Copyright 2015-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdint.h>
#include "esp_flash.h"
#include "spi_flash_chip_driver.h"


/*
 * The 'chip_generic' SPI flash operations are a lowest common subset of SPI
 * flash commands, that work across most chips.
 *
 * These can be used as-is via the esp_flash_common_chip_driver chip_drv, or
 * they can be used as "base chip_drv" functions when creating a new
 * spi_flash_host_driver_t chip_drv structure.
 *
 * All of the functions in this header are internal functions, not part of a
 * public API. See esp_flash.h for the public API.
 */

/**
 * @brief Generic probe function
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param flash_id expected manufacture id.
 *
 * @return ESP_OK if the id read from chip->drv_read_id matches (always).
 */
esp_err_t spi_flash_chip_generic_probe(esp_flash_t *chip, uint32_t flash_id);

/**
 * @brief Generic reset function
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return ESP_OK if sending success, or error code passed from ``common_command`` or ``wait_idle`` functions of host driver.
 */
esp_err_t spi_flash_chip_generic_reset(esp_flash_t *chip);

/**
 * @brief Generic size detection function
 *
 * Tries to detect the size of chip by using the lower 4 bits of the chip->drv->read_id result = N, and assuming size is 2 ^ N.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param size Output of the detected size
 *
 * @return
 *      - ESP_OK if success
 *      - ESP_ERR_FLASH_UNSUPPORTED_CHIP if the manufacturer id is not correct, which may means an error in the reading
 *      - or other error passed from the ``read_id`` function of host driver
 */
esp_err_t spi_flash_chip_generic_detect_size(esp_flash_t *chip, uint32_t *size);

/**
 * @brief Erase chip by using the generic erase chip (C7h) command.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``set_write_protect``, ``wait_idle`` or ``erase_chip`` function of host driver
 */
esp_err_t spi_flash_chip_generic_erase_chip(esp_flash_t *chip);

/**
 * @brief Erase sector by using the generic sector erase (20h) command.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param start_address Start address of the sector to erase
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``set_write_protect``, ``wait_idle`` or ``erase_sector`` function of host driver
 */
esp_err_t spi_flash_chip_generic_erase_sector(esp_flash_t *chip, uint32_t start_address);

/**
 * @brief Erase block by using the generic 64KB block erase (D8h) command
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param start_address Start address of the block to erase
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``set_write_protect``, ``wait_idle`` or ``erase_block`` function of host driver
 */
esp_err_t spi_flash_chip_generic_erase_block(esp_flash_t *chip, uint32_t start_address);

/**
 * @brief Read from flash by using a read command that matches the programmed
 * read mode.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param buffer Buffer to hold the data read from flash
 * @param address Start address of the data on the flash
 * @param length Length to read
 *
 * @return always ESP_OK currently
 */
esp_err_t spi_flash_chip_generic_read(esp_flash_t *chip, void *buffer, uint32_t address, uint32_t length);

/**
 * @brief Perform a page program using the page program (02h) command.
 *
 * @note Length of each call should not excced the limitation in
 * ``chip->host->max_write_bytes``. This function is called in
 * ``spi_flash_chip_generic_write`` recursively until the whole page is
 * programmed. Strongly suggest to call ``spi_flash_chip_generic_write``
 * instead.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param buffer Buffer holding the data to program
 * @param address Start address to write to flash
 * @param length Length to write, no longer than ``chip->host->max_write_bytes``.
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``wait_idle`` or ``program_page`` function of host driver
 */
esp_err_t
spi_flash_chip_generic_page_program(esp_flash_t *chip, const void *buffer, uint32_t address, uint32_t length);

/**
 * @brief Perform a generic write. Split the write buffer into page program
 * operations, and call chip->chip_drv->page-program() for each.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param buffer Buffer holding the data to program
 * @param address Start address to write to flash
 * @param length Length to write
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``wait_idle`` or ``program_page`` function of host driver
  */
esp_err_t spi_flash_chip_generic_write(esp_flash_t *chip, const void *buffer, uint32_t address, uint32_t length);

/**
 * @brief Perform a write using on-chip flash encryption. Not implemented yet.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param buffer Buffer holding the data to program
 * @param address Start address to write to flash
 * @param length Length to write
 *
 * @return always ESP_ERR_FLASH_UNSUPPORTED_HOST.
 */
esp_err_t
spi_flash_chip_generic_write_encrypted(esp_flash_t *chip, const void *buffer, uint32_t address, uint32_t length);

/**
 * @brief Send the write enable (06h) command and verify the expected bit (1) in
 * the status register is set.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param write_protect true to enable write protection, false to send write enable.
 *
 * @return
 *      - ESP_OK if success
 *      - or other error passed from the ``wait_idle``, ``read_status`` or ``set_write_protect`` function of host driver
 */
esp_err_t spi_flash_chip_generic_write_enable(esp_flash_t *chip, bool write_protect);

/**
 * @brief Read flash status via the RDSR command (05h) and wait for bit 0 (write
 * in progress bit) to be cleared.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 * @param timeout_ms Time to wait before timeout, in ms.
 *
 * @return
 *      - ESP_OK if success
 *      - ESP_ERR_TIMEOUT if not idle before timeout
 *      - or other error passed from the ``wait_idle`` or ``read_status`` function of host driver
 */
esp_err_t spi_flash_chip_generic_wait_idle(esp_flash_t *chip, uint32_t timeout_ms);

/**
 * @brief Set the specified SPI read mode according to the data in the chip
 *        context. Set quad enable status register bit if needed.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return
 *      - ESP_OK if success
*      - ESP_ERR_TIMEOUT if not idle before timeout
 *      - or other error passed from the ``set_write_protect`` or ``common_command`` function of host driver
 */
esp_err_t spi_flash_chip_generic_set_read_mode(esp_flash_t *chip);

/**
 * Generic SPI flash chip_drv, uses all the above functions for its operations.
 * In default autodetection, this is used as a catchall if a more specific
 * chip_drv is not found.
 */
extern const spi_flash_chip_t esp_flash_chip_generic;

/*******************************************************************************
 *  Utilities
*******************************************************************************/

/**
 * @brief Wait for the SPI host hardware state machine to be idle.
 *
 * This isn't a flash chip_drv operation, but it's called by
 * spi_flash_chip_generic_wait_idle() and may be useful when implementing
 * alternative drivers.
 *
 * timeout_ms will be decremented if the function needs to wait until the host hardware is idle.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return
 *      - ESP_OK if success
 *      - ESP_ERR_TIMEOUT if not idle before timeout
 *      - or other error passed from the ``set_write_protect`` or ``common_command`` function of host driver
 */
esp_err_t spi_flash_generic_wait_host_idle(esp_flash_t *chip, uint32_t *timeout_ms);

/**
 * @brief Utility function for set_read_mode chip_drv function
 *
 * Most setting of read mode follows a common pattern, except for how to enable Quad I/O modes (QIO/QOUT).
 * These use different commands to read/write the status register, and a different bit is set/cleared.
 *
 * This is a generic utility function to implement set_read_mode() for this pattern. Also configures host
 * registers via spi_flash_common_configure_host_read_mode().
 *
 * @param qe_rdsr_command SPI flash command to read status register
 * @param qe_wrsr_command SPI flash command to write status register
 * @param qe_sr_bitwidth Width of the status register these commands operate on, in bits.
 * @param qe_sr_bit Bit mask for enabling Quad Enable functions on this chip.
 *
 * @return always ESP_OK (currently).
 */
esp_err_t spi_flash_common_set_read_mode(esp_flash_t *chip, uint8_t qe_rdsr_command, uint8_t qe_wrsr_command, uint8_t qe_sr_bitwidth, unsigned qe_sr_bit);

/**
 * @brief Configure the host to use the specified read mode set in the ``chip->read_mode``.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return
 *      - ESP_OK if success
 *      - ESP_ERR_FLASH_NOT_INITIALISED if chip not initialized properly
 *      - or other error passed from the ``configure_host_mode`` function of host driver
 */
esp_err_t spi_flash_chip_generic_config_host_read_mode(esp_flash_t *chip);

/**
 * @brief Returns true if chip is configured for Quad I/O or Quad Fast Read.
 *
 * @param chip Pointer to SPI flash chip to use. If NULL, esp_flash_default_chip is substituted.
 *
 * @return true if flash works in quad mode, otherwise false
 */
static inline bool spi_flash_is_quad_mode(const esp_flash_t *chip)
{
    return (chip->read_mode == SPI_FLASH_QIO) || (chip->read_mode == SPI_FLASH_QOUT);
}

