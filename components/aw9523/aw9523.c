#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "aw9523.h"

i2c_master_bus_handle_t i2c_bus_handle;
static i2c_master_dev_handle_t aw9523_dev_handle;

#define AW9523_CHECK_NUM(pin_num) if ((pin_num) > 7) { return; }

void aw9523_init(uint8_t sda_pin, uint8_t scl_pin) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = scl_pin,
        .sda_io_num = sda_pin,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x5b,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &aw9523_dev_handle));
}

uint8_t aw9523_read_level(aw9523_port_t port) {
    uint8_t reg = (port == AW9523_PORT_0) ? 0x00 : 0x01;
    uint8_t data = 0x00;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &data, 1, -1));
    return data;
}

void aw9523_set_level(aw9523_port_t port, uint8_t value) {
    uint8_t reg = (port == AW9523_PORT_0) ? 0x02 : 0x03;
    uint8_t write_buf[2] = {reg, value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_io_set_level(aw9523_port_t port, uint8_t pin_num, uint8_t value) {
    AW9523_CHECK_NUM(pin_num);
    uint8_t reg = (port == AW9523_PORT_0) ? 0x02 : 0x03;
    uint8_t current_value = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &current_value, 1, -1));
    
    if (value > 0) {
        current_value |= (1 << pin_num);
    } else {
        current_value &= ~(1 << pin_num);
    }
    
    uint8_t write_buf[2] = {reg, current_value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_set_inout(aw9523_port_t port, uint8_t mode) {
    uint8_t reg = (port == AW9523_PORT_0) ? 0x04 : 0x05;
    uint8_t write_buf[2] = {reg, mode};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_io_set_inout(aw9523_port_t port, uint8_t pin_num, aw9523_inout_mode_t mode) {
    AW9523_CHECK_NUM(pin_num);
    uint8_t reg = (port == AW9523_PORT_0) ? 0x04 : 0x05;
    uint8_t current_value = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &current_value, 1, -1));
    
    uint8_t bit_value = (mode == AW9523_MODE_INPUT) ? 1 : 0;
    if (bit_value) {
        current_value |= (1 << pin_num);
    } else {
        current_value &= ~(1 << pin_num);
    }
    
    uint8_t write_buf[2] = {reg, current_value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_set_port0_pp(uint8_t pp_enable) {
    uint8_t reg = 0x11;
    uint8_t current_value = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &current_value, 1, -1));
    
    if (pp_enable) {
        current_value |= (1 << 4);
    } else {
        current_value &= ~(1 << 4);
    }
    
    uint8_t write_buf[2] = {reg, current_value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_set_led_max_current(aw9523_current_t current) {
    uint8_t reg = 0x11;
    uint8_t current_value = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &current_value, 1, -1));
    
    current_value &= ~0x03;  // Clear bits 0-1
    current_value |= (current & 0x03);  // Set new current value
    
    uint8_t write_buf[2] = {reg, current_value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_set_gpio_or_led(aw9523_port_t port, uint8_t mode) {
    uint8_t reg = (port == AW9523_PORT_0) ? 0x12 : 0x13;
    uint8_t write_buf[2] = {reg, mode};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_io_set_gpio_or_led(aw9523_port_t port, uint8_t pin_num, aw9523_mode_t mode) {
    AW9523_CHECK_NUM(pin_num);
    uint8_t reg = (port == AW9523_PORT_0) ? 0x12 : 0x13;
    uint8_t current_value = 0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(aw9523_dev_handle, &reg, 1, &current_value, 1, -1));
    
    uint8_t bit_value = (mode == AW9523_MODE_GPIO) ? 1 : 0;
    if (bit_value) {
        current_value |= (1 << pin_num);
    } else {
        current_value &= ~(1 << pin_num);
    }
    
    uint8_t write_buf[2] = {reg, current_value};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_led_set_duty(aw9523_port_t port, uint8_t pin_num, uint8_t duty) {
    AW9523_CHECK_NUM(pin_num);
    uint8_t reg;
    if (port == AW9523_PORT_0) {
        reg = 0x24 + pin_num;
    } else {
        reg = 0x20 + pin_num + ((pin_num > 3) ? 0x08 : 0x00);
    }
    uint8_t write_buf[2] = {reg, duty};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}

void aw9523_leds_set_duty(aw9523_port_t port, uint8_t pin_num, uint8_t nums, uint8_t duty) {
    AW9523_CHECK_NUM(pin_num + nums);
    uint8_t dutys[8] = {0};
    memset(dutys, duty, 8);
    uint8_t reg;
    if (port == AW9523_PORT_0) {
        reg = 0x24 + pin_num;
        uint8_t write_buf[9];
        write_buf[0] = reg;
        memcpy(&write_buf[1], dutys, nums);
        ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, nums + 1, -1));
    } else {
        if (pin_num < 4) {
            uint8_t need_write = 4 - pin_num;
            if (need_write > nums) {
                need_write = nums;
            }
            nums -= need_write;
            uint8_t write_buf[9];
            write_buf[0] = 0x20 + pin_num;
            memcpy(&write_buf[1], dutys, need_write);
            ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, need_write + 1, -1));
            pin_num = 4;
        }
        if (nums) {
            uint8_t write_buf[9];
            write_buf[0] = 0x20 + 0x08 + pin_num;
            memcpy(&write_buf[1], dutys, nums);
            ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, nums + 1, -1));
        }
    }
}

void aw9523_softreset() {
    uint8_t write_buf[2] = {0x7f, 0x00};
    ESP_ERROR_CHECK(i2c_master_transmit(aw9523_dev_handle, write_buf, 2, -1));
}







