#ifndef __LCD_BACKLIGHT_H__
#define __LCD_BACKLIGHT_H__

#include "stdint.h"

/*
 * Note: Ensure settings align with the hardware configuration.
 * On original hardware, LCD_BL_ON_LEVEL must be set to 0.
 * Be advised: This may trigger visible stripes on specific display models during the boot process.
 */
#define LCD_BL_ON_LEVEL     (1)

#define BACKLIGHT_PWM_ENABLE (1)
#if BACKLIGHT_PWM_ENABLE
#define BACKLIGHT_PWM_FREQUENCY (1000)
#define BACKLIGHT_PWM_DUTY_RESOLUTION LEDC_TIMER_13_BIT
#define BACKLIGHT_PWM_MAX_DUTY (8192)
#define BACKLIGHT_PWM_CHANNEL LEDC_CHANNEL_0
#define BACKLIGHT_PWM_TIMER LEDC_TIMER_0
#define BACKLIGHT_PWM_CLK_SRC LEDC_USE_RC_FAST_CLK
#define BACKLIGHT_PWM_SPEED_MODE LEDC_LOW_SPEED_MODE
#endif

void lcd_backlight_init(uint8_t gpio_bl);
void lcd_backlight_onoff(uint8_t onoff);
uint8_t lcd_backlight_get_pwm_duty(void);
void lcd_backlight_set_pwm_duty(uint8_t duty);

#endif
