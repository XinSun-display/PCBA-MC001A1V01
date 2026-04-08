#ifndef __CTP_GT9XX_H__
#define __CTP_GT9XX_H__

#define GT911_PRODUCT_ID_REG	    0x8140
#define GT911_CONFIG_REG		    0x8047
#define GT911_FIRMWARE_VERSION_REG  0x8144 
#define GT911_READ_XY_STATUS_REG 	0x814E
#define GT911_READ_XY_REG 			0x8150

void GT911_Init(void);
void GT915_Init(void);
void GT9XX_Init(uint16_t dev_addr);
esp_err_t GT9XX_read_Reg(uint16_t reg,uint8_t *buf,uint8_t len);

void GT911_Scan(lv_indev_t *indev_drv, lv_indev_data_t *data);
#define GT915_Scan GT911_Scan

#endif