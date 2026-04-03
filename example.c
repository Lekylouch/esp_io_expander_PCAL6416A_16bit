/*
 * PCAL6416A minimal example for ESP-IDF v5
 *
 * This example shows a complete minimal setup for using the
 * esp_io_expander_pcal6416a_16bit driver with the ESP-IDF v5
 * I2C master bus API.
 *
 * I had some difficulty configuring the project because there was
 * no clear end-to-end example. This file is intended to provide
 * a simple working reference for anyone who wants to use this driver.
 *
 * Tested with:
 * - ESP32
 * - ESP-IDF v5.x
 * - SDA: GPIO21
 * - SCL: GPIO22
 * - PCAL6416A address: 0x20
 *
 * Behavior:
 * - Configures pins 14 and 15 as outputs
 * - Toggles both pins every 1 second
 */




#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_io_expander_pcal6416a_16bit.h"

static const char *TAG = "PCAL6416A_EXAMPLE";

// --- Hardware configuration ---
#define I2C_MASTER_SCL_IO           22      // GPIO pin used for I2C clock (SCL)
#define I2C_MASTER_SDA_IO           21      // GPIO pin used for I2C data (SDA)
#define I2C_MASTER_PORT             0       // I2C port number
#define IO_EXPANDER_I2C_ADDR        0x20    // I2C address of the PCAL6416A IO expander

// Pin mask to control (here pins 14 and 15)
#define LED_PINS_MASK               (IO_EXPANDER_PIN_NUM_14 | IO_EXPANDER_PIN_NUM_15)

// Global handles
esp_io_expander_handle_t io_expander = NULL;
i2c_master_bus_handle_t i2c_bus = NULL;

void app_main(void)
{
    ESP_LOGI(TAG, "Starting PCAL6416A IO expander example");

    // 1) Configure and initialize the I2C bus (ESP-IDF v5+ I2C master driver)
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_LOGI(TAG, "Initializing I2C bus...");
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus));

    // 2) Create the IO expander instance
    ESP_LOGI(TAG, "Connecting to IO expander (address: 0x%02X)...", IO_EXPANDER_I2C_ADDR);
    esp_err_t err = esp_io_expander_new_i2c_pcal6416a_16bit(
        i2c_bus,
        IO_EXPANDER_I2C_ADDR,
        &io_expander
    );

    if (err != ESP_OK || io_expander == NULL) {
        ESP_LOGE(TAG, "Failed to create IO expander: %s", esp_err_to_name(err));
        return; // Stop execution if the device is not detected
    }

    ESP_LOGI(TAG, "IO expander initialized successfully");

    // 3) Configure selected pins as outputs
    ESP_ERROR_CHECK(esp_io_expander_set_dir(
        io_expander,
        LED_PINS_MASK,
        IO_EXPANDER_OUTPUT
    ));

    bool level = false;

    // 4) Main loop: toggle pins 14 and 15
    while (1) {
        ESP_ERROR_CHECK(esp_io_expander_set_level(
            io_expander,
            LED_PINS_MASK,
            level
        ));

        ESP_LOGI(TAG, "Pins 14 and 15 set to: %d", level);

        level = !level;                  // Toggle state for next cycle
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
    }
}
