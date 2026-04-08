#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "aw9523.h"
#include "lvgl.h"
#include "CTP_gt9xx.h"

#define GT9XX_SCL   47
#define GT9XX_SDA   48

#define GT911_ADDR   0xBA // 0xBA or 0xBB
#define GT915_ADDR   0x5d // 0x14 or 0x5d

static i2c_master_dev_handle_t GT9XX_dev_handle;

static const char *TAG = "GT9XX";

esp_err_t GT9XX_write_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
    ESP_RETURN_ON_FALSE(buf != NULL, ESP_ERR_INVALID_ARG, TAG, "I2C send Buffer is NULL");
    ESP_RETURN_ON_FALSE(len <= 19, ESP_ERR_INVALID_ARG, TAG, "I2C send Length is too long");

    static uint8_t new_reg[20];
    len = len > 18 ? 18 : len;
    new_reg[0] = (reg&0xFF00)>>8;
    new_reg[1] = (reg&0x00FF);
    for (int i = 0; i < len; i++) {
        new_reg[i+2] = buf[i];
    }
    ESP_ERROR_CHECK(i2c_master_transmit(GT9XX_dev_handle, (uint8_t*)new_reg, len+2, -1));

	return ESP_OK;
}

/**
 * @brief Read register from GT9XX
 * 
 * @param reg Register address
 * @param buf Buffer to read
 * @param len Length of buffer
 * @return ESP_OK on success, otherwise error code
 */
esp_err_t GT9XX_read_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
    ESP_RETURN_ON_FALSE(buf != NULL, ESP_ERR_INVALID_ARG, TAG, "I2C receive Buffer is NULL");

    uint8_t reg8_buf[2];
    reg8_buf[0] = (reg&0xFF00)>>8;
    reg8_buf[1] = (reg&0x00FF);
    ESP_ERROR_CHECK(i2c_master_transmit_receive(GT9XX_dev_handle, reg8_buf, 2,  (uint8_t*)buf, len, -1));

    return ESP_OK;
}

/**
 * @brief Initialize GT911
 * 
 * @return None
 */
void GT911_Init(void)
{
    GT9XX_Init(GT911_ADDR);
}

/**
 * @brief Initialize GT915
 * 
 * @return None
 */
void GT915_Init(void)
{
    GT9XX_Init(GT915_ADDR);
}

/**
 * @brief Initialize GT9XX
 * 
 * @return None
 */
void GT9XX_Init(uint16_t dev_addr)
{
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(I2C_NUM_0, &i2c_bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &GT9XX_dev_handle));

    //init CTP_INT PIN(GPIO4)
    gpio_config_t config = {.pin_bit_mask = (1ULL << GPIO_NUM_4),
                            .mode = GPIO_MODE_OUTPUT,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(GPIO_NUM_4, 0));

    // hw reset
    aw9523_io_set_level(AW9523_PORT_1,1,1) ;
    vTaskDelay(20 / portTICK_PERIOD_MS);  
    aw9523_io_set_level(AW9523_PORT_1,1,0) ;
    vTaskDelay(120 / portTICK_PERIOD_MS);  
	aw9523_io_set_level(AW9523_PORT_1,1,1) ;
    vTaskDelay(120 / portTICK_PERIOD_MS);  

    uint8_t buf[4];
    GT9XX_read_Reg(GT911_PRODUCT_ID_REG, (uint8_t *)&buf, 3);
	GT9XX_read_Reg(GT911_CONFIG_REG, (uint8_t *)&buf[3], 1);
	ESP_LOGI(TAG, "TouchPad_ID:%c,%c,%c TouchPad_Config_Version:%2x",buf[0],buf[1],buf[2],buf[3]);
	GT9XX_read_Reg(GT911_FIRMWARE_VERSION_REG, (uint8_t *)&buf, 2);
	ESP_LOGI(TAG, "FirmwareVersion:%2x",(((uint16_t)buf[1] << 8) + buf[0]));
}

void GT911_Scan(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
	uint8_t status_reg = 0;
    uint8_t temp[4];

    /*Read status register*/
    GT9XX_read_Reg(GT911_READ_XY_STATUS_REG,&status_reg,1);
    
    if ((status_reg & 0x80) || (status_reg & 0xF) <= 5) 
    {
        //Reset Status Reg Value
        temp[0] = 0x00;
        GT9XX_write_Reg(GT911_READ_XY_STATUS_REG, temp, 1);
    }

    if ((status_reg & 0xF) && ((status_reg & 0xF) <= 5))//has touch and less than 5 points
    {
        GT9XX_read_Reg(GT911_READ_XY_REG,temp,4);
        data->state = LV_INDEV_STATE_PR;
        data->point.x = ((uint16_t)(temp[1] & 0X0F) << 8) + temp[0];
        data->point.y = ((uint16_t)(temp[3] & 0X0F) << 8) + temp[2];
        // ESP_LOGI(TAG, "touch: x=%ld, y=%ld\r\n", data->point.x, data->point.y);
    }	
    else
    {
            data->state = LV_INDEV_STATE_REL;
    }
			
}	