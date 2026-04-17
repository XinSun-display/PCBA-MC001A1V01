#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "aw9523.h"
#include "lvgl.h"
#include "CTP_FT5446.h"
#include "aw9523.h"

#define FT5446_SCL   47
#define FT5446_SDA   48

#define CTP_WRITE             			0x70
#define CTP_READ             			0x71

#define X_LENGTH 800
#define Y_LENGTH 480

static i2c_master_dev_handle_t FT5446_dev_handle;

static const char *TAG = "FT5446";

/**
 * @brief Write register to FT5446
 * 
 * @param reg Register address
 * @param buf Buffer to write
 * @param len Length of buffer
 * @return ESP_OK on success, otherwise error code
 */
esp_err_t FT5XXX_WR_Reg(uint8_t reg,uint8_t *buf,uint8_t len)
{
    ESP_RETURN_ON_FALSE(buf != NULL, ESP_ERR_INVALID_ARG, TAG, "I2C send Buffer is NULL");
    ESP_RETURN_ON_FALSE(len <= 19, ESP_ERR_INVALID_ARG, TAG, "I2C send Length is too long");

    static U8 new_reg[20];
    len = len > 19 ? 19 : len;
    new_reg[0] = reg;
    for (int i = 0; i < len; i++) {
        new_reg[i+1] = buf[i];
    }
    ESP_ERROR_CHECK(i2c_master_transmit(FT5446_dev_handle, (uint8_t*)new_reg, len+1, -1));

	return ESP_OK;
}

/**
 * @brief Read register from FT5446
 * 
 * @param reg Register address
 * @param value Buffer to read
 * @param len Length of buffer
 * @return ESP_OK on success, otherwise error code
 */
esp_err_t FT5XXX_Reg(U8 reg, U8 *value, U8 len) 
{
    ESP_RETURN_ON_FALSE(value != NULL, ESP_ERR_INVALID_ARG, TAG, "I2C receive Buffer is NULL");

    uint8_t buf[2];
    buf[0] = reg;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(FT5446_dev_handle, buf, 1,  (uint8_t*)value, len, -1));

	return ESP_OK;
}

/**
 * @brief Initialize FT5446
 * 
 * @return None
 */
void FT5446_Init(void)
{
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(I2C_NUM_0, &i2c_bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x38,
        .scl_speed_hz = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &FT5446_dev_handle));

    // hw reset
	aw9523_io_set_level(AW9523_PORT_1,1,1) ;
    vTaskDelay(20 / portTICK_PERIOD_MS);  
    aw9523_io_set_level(AW9523_PORT_1,1,0) ;
    vTaskDelay(120 / portTICK_PERIOD_MS);  
	aw9523_io_set_level(AW9523_PORT_1,1,1) ;
    vTaskDelay(120 / portTICK_PERIOD_MS);  

    static unsigned char temp[4];

    temp[0] = 0x00;
    FT5XXX_WR_Reg(FT5XXX_OP_DEVIDE_MODE, temp, 1);     

    temp[0] = 0xff;
    FT5XXX_WR_Reg(FT5XXX_OP_GEST_ID, temp, 1);   

    temp[0] = 0xa;
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_THCAL, temp, 1);   

    temp[0] = 0xa;
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_THWATER, temp, 1); 

    temp[0] = 0xa;
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_THTEMP, temp, 1); 

    temp[0] = 1;
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_MODE, temp, 1);    

    temp[0] = 22;                                   
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_THGROUP, temp, 1);    

    temp[0] = 12;                                  
    FT5XXX_WR_Reg(FT5XXX_OP_ID_G_PERIODACTIVE, temp, 1);

    FT5XXX_Reg(FT5XXX_OP_DEVIDE_MODE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_DEVIDE_MODE(0x00):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_GEST_ID, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_GEST_ID(0x01):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_TD_STATUS, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_TD_STATUS(0x02):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THGROUP, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THGROUP(0x80):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THPEAK, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THPEAK(0x81):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THCAL, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THCAL(0x82):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THWATER, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THWATER(0x83):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THTEMP, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THTEMP(0x84):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_THTDIFF, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_THTDIFF(0x85):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_CTRL, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_CTRL(0x86):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_TIMMONITOR, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_TIMMONITOR(0x87):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_PERIODACTIVE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_PERIODACTIVE(0x88):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_PERIODMONITOR, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_PERIODMONITOR(0x89):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_L_R_OFFSET, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_L_R_OFFSET(0x92):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_U_D_OFFSET, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_U_D_OFFSET(0x93):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_DISTANCE_LEFT_RIGHT, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_DISTANCE_LEFT_RIGHT(0x94):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_DISTANCE_UP_DOWN, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_DISTANCE_UP_DOWN(0x95):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_RADIAN_VALUE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_RADIAN_VALUE(0x96):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_ZOOM_DIS_SQR, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_ZOOM_DIS_SQR(0x97):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_MAX_X_HIGH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_MAX_X_HIGH(0x98):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_MAX_X_LOW, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_MAX_X_LOW(0x99):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_MAX_Y_HIGH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_MAX_Y_HIGH(0x9A):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_MAX_Y_LOW, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_MAX_Y_LOW(0x9B):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_K_X_HIGH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_K_X_HIGH(0x9C):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_K_X_LOW, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_K_X_LOW(0x9D):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_K_Y_HIGH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_K_Y_HIGH(0x9E):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_K_Y_LOW, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_K_Y_LOW(0x9F):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_AUTOCLBMODE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_AUTOCLBMODE(0xA0):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_LIBVERSIONH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_LIBVERSIONH(0xA1):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_LIBVERSIONL, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_LIBVERSIONL(0xA2):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_CIPHER, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_CIPHER(0xA3):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_MODE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_MODE(0xA4):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_PMODE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_PMODE(0xA5):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_FIRMID, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_FIRMID(0xA6):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_STATE, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_STATE(0xA7):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_ERR, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_ERR(0xA9):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_CLB, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_CLB(0xAA):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_STATIC_TH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_STATIC_TH(0xAB):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_AUTOCLB_STATUS, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_AUTOCLB_STATUS(0xAC):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_AUTOCLB_TIMER, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_AUTOCLB_TIMER(0xAD):0x%x\r\n", temp[0]);

    FT5XXX_Reg(FT5XXX_OP_ID_G_DRAW_LINE_TH, temp, 1);
    ESP_LOGD(TAG, "FT5XXX_OP_ID_G_DRAW_LINE_TH(0xAE):0x%x\r\n", temp[0]);
}

/**
 * @brief Scan touch screen V1 version, for lvgl 
 * @param indev_drv Input device driver
 * @param data Touch data
 * @return None
 */
void FT5446_ScanV1(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    uint8_t mode = 0;
    uint8_t temp[4];

    /*Read status register*/
    FT5XXX_Reg(FT5XXX_OP_TD_STATUS,&mode,1);
    
    if ((mode & 0xF) && ((mode & 0xF) <= 10))//has touch and less than 10 points
    {
        FT5XXX_Reg(FT5XXX_OP_TP1_REG,temp,4);
        data->state = LV_INDEV_STATE_PR;
        // ESP_LOGI(TAG, "touch: x=%ld, y=%ld\r\n", data->point.x, data->point.y);
    }	
    else
    {
            data->state = LV_INDEV_STATE_REL;

    }
    data->point.x = ((uint16_t)(temp[0] & 0X0F) << 8) + temp[1];
    data->point.y = ((uint16_t)(temp[2] & 0X0F) << 8) + temp[3];

    return;
}

/**
 * @brief Scan touch screen V2 version, for lvgl 
 * @param indev_drv Input device driver
 * @param data Touch data
 * @return None
 */
void FT5446_ScanV2(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    uint8_t mode = 0;
    uint8_t temp[5];

    /*Read status register*/
    FT5XXX_Reg(FT5XXX_OP_TD_STATUS,&mode,1);
    FT5XXX_Reg(FT5XXX_OP_TP1_REG,temp,1);
    FT5XXX_Reg(FT5XXX_OP_TP1_REG+1,temp+1,1);
    FT5XXX_Reg(FT5XXX_OP_TP1_REG+2,temp+2,1);
    FT5XXX_Reg(FT5XXX_OP_TP1_REG+3,temp+3,1);
    FT5XXX_Reg(FT5XXX_OP_TP1_REG+4,temp+4,1);
    data->point.x = ((uint16_t)(temp[1] & 0X0F) << 8) + temp[2];
    data->point.y = ((uint16_t)(temp[3] & 0X0F) << 8) + temp[4];
    if (temp[0] > 0)//has touch?
    {
        data->state = LV_INDEV_STATE_PR;      
        // ESP_LOGI(TAG, "touch: x=%ld, y=%ld\r\n", data->point.x, data->point.y);
    }	
    else
    {
         data->state = LV_INDEV_STATE_REL;

    }

    return;
}