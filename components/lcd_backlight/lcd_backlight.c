#include "lcd_backlight.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

int lcd_gpio_bl = -1;

static const char *TAG = "lcd_backlight";

/*  *
 * @brief Initialize LCD backlight
 * 
 * @param gpio_bl GPIO number for backlight
 * @return None
 */
void lcd_backlight_init(uint8_t gpio_bl)
{

    if(LCD_BL_ON_LEVEL!=0)
    {
        ESP_LOGW(TAG, "LCD_BL_ON_LEVEL is not 0, please check the hardware configuration. On original hardware, LCD_BL_ON_LEVEL must be set to 0. ");
    }

    #if BACKLIGHT_PWM_ENABLE
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = BACKLIGHT_PWM_SPEED_MODE,
        .duty_resolution  = BACKLIGHT_PWM_DUTY_RESOLUTION ,
        .timer_num        = BACKLIGHT_PWM_TIMER,
        .freq_hz          = BACKLIGHT_PWM_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = BACKLIGHT_PWM_CLK_SRC,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = BACKLIGHT_PWM_SPEED_MODE,
        .channel        = BACKLIGHT_PWM_CHANNEL,
        .timer_sel      = BACKLIGHT_PWM_TIMER,
        .gpio_num       = gpio_bl,
        .duty           = BACKLIGHT_PWM_MAX_DUTY, // Set duty to 0%
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    #else
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << gpio_bl
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    #endif

    lcd_gpio_bl = gpio_bl;
    
}

/**
 * @brief Control LCD backlight on/off
 * 
 * @param onoff 1: on, 0: off
 * @return None
 */
void lcd_backlight_onoff(uint8_t onoff)
{
    if(lcd_gpio_bl == -1)
    {
        return;
    }

#if BACKLIGHT_PWM_ENABLE
    if(onoff)
    {
        lcd_backlight_set_pwm_duty(100);
    }
    else
    {
        lcd_backlight_set_pwm_duty(0);
    }
#else
    ESP_ERROR_CHECK(gpio_set_level(lcd_gpio_bl, onoff?LCD_BL_ON_LEVEL:!LCD_BL_ON_LEVEL));
#endif
}


#if BACKLIGHT_PWM_ENABLE
/**
 * @brief 
 * 
 * 
 * @return Duty cycle (0-100)
 */
uint8_t lcd_backlight_get_pwm_duty(void)
{
    return ledc_get_duty(BACKLIGHT_PWM_SPEED_MODE, BACKLIGHT_PWM_CHANNEL)*100/BACKLIGHT_PWM_MAX_DUTY;
}

/**
 * @brief Set LCD backlight PWM duty
 * 
 * @param duty Duty cycle (0-100)
 * @return None
 */
void lcd_backlight_set_pwm_duty(uint8_t duty)
{
    uint8_t set_duty = LCD_BL_ON_LEVEL?duty:100-duty;
    ESP_ERROR_CHECK(ledc_set_duty(BACKLIGHT_PWM_SPEED_MODE, BACKLIGHT_PWM_CHANNEL, set_duty*BACKLIGHT_PWM_MAX_DUTY/100));
    ESP_ERROR_CHECK(ledc_update_duty(BACKLIGHT_PWM_SPEED_MODE, BACKLIGHT_PWM_CHANNEL));  
}
#endif