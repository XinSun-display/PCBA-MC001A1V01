#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_lvgl_port.h"

typedef struct
{ 
  uint8_t hbp;  //HSYNC back porch
  uint8_t vbp;  //VSYNC back porch

  uint8_t hsw;   //HSYNC pulse width
  uint8_t vsw;   //VSYNC pulse width

  uint8_t hfp;  	//HSYNC front porch
  uint8_t vfp;  	//VSYNC front porch
  
  uint32_t clock_HZ; //LCD clock frequency

  uint16_t lcd_pixel_width; //LCD pixel width
  uint16_t lcd_pixel_height;//LCD pixel height
  void (*CTP_init)(void);
  void (*CTP_scan)(lv_indev_t *indev_drv, lv_indev_data_t *data);
}LCD_PARAM_TypeDef;

/* Parameters for different LCDs */
extern const LCD_PARAM_TypeDef lcd_param[];

#define LCD_HBP  lcd_param[CUR_LCD].hbp		//HSYNC back porch
#define LCD_VBP  lcd_param[CUR_LCD].vbp		//VSYNC back porch

#define LCD_HSW  lcd_param[CUR_LCD].hsw		//HSYNC pulse width
#define LCD_VSW  lcd_param[CUR_LCD].vsw		//VSYNC pulse width

#define LCD_HFP  lcd_param[CUR_LCD].hfp		//HSYNC front porch
#define LCD_VFP  lcd_param[CUR_LCD].vfp		//VSYNC front porch

#define LCD_CLK_HZ  lcd_param[CUR_LCD].clock_HZ		//LCD clock frequency

/* LCD Size (Width and Height) */
#define  LCD_PIXEL_WIDTH          lcd_param[CUR_LCD].lcd_pixel_width
#define  LCD_PIXEL_HEIGHT         lcd_param[CUR_LCD].lcd_pixel_height

/* CTP init and scan function pointer */
#define CTP_INIT (*lcd_param[CUR_LCD].CTP_init)
#define CTP_SCAN (*lcd_param[CUR_LCD].CTP_scan)

extern void display_init(void);

#ifdef __cplusplus
}
#endif

#endif/* __DISPLAY_H */