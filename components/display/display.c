#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "display.h"
#include "aw9523.h"
#include "CTP_FT5446.h"
#include "CTP_gt9xx.h"
#include "ui.h"

/* LCD settings */
#define LCD_COLOR_SPACE     (ESP_LCD_COLOR_SPACE_BGR)//(ESP_LCD_COLOR_SPACE_RGB)
#define LCD_BITS_PER_PIXEL  (16)
#define LCD_DRAW_BUFF_DOUBLE (1)
#define LCD_DRAW_BUFF_HEIGHT (16)

/*
 * Note: Ensure settings align with the hardware configuration.
 * On original hardware, LCD_BL_ON_LEVEL must be set to 0.
 * Be advised: This may trigger visible stripes on specific display models during the boot process.
 */
#define LCD_BL_ON_LEVEL     (0)

/* LCD pins */
#define LCD_GPIO_BL (GPIO_NUM_45)
#define LCD_RGB_D15 (GPIO_NUM_14)
#define LCD_RGB_D14 (GPIO_NUM_13)
#define LCD_RGB_D13 (GPIO_NUM_12)
#define LCD_RGB_D12 (GPIO_NUM_11)
#define LCD_RGB_D11 (GPIO_NUM_10)
#define LCD_RGB_D10 (GPIO_NUM_18)
#define LCD_RGB_D9 (GPIO_NUM_8)
#define LCD_RGB_D8 (GPIO_NUM_3)
#define LCD_RGB_D7 (GPIO_NUM_46)
#define LCD_RGB_D6 (GPIO_NUM_0)
#define LCD_RGB_D5 (GPIO_NUM_21)
#define LCD_RGB_D4 (GPIO_NUM_6)
#define LCD_RGB_D3 (GPIO_NUM_7)
#define LCD_RGB_D2 (GPIO_NUM_15)
#define LCD_RGB_D1 (GPIO_NUM_16)
#define LCD_RGB_D0 (GPIO_NUM_17)
#define LCD_RGB_PCLK (GPIO_NUM_9)
#define LCD_RGB_HS (GPIO_NUM_5)
#define LCD_RGB_VS (GPIO_NUM_38)
#define LCD_RGB_DE (GPIO_NUM_39)
#define LCD_RGB_DISP (-1)

static const char *TAG = "DISPLAY";

/* LCD panel */
static esp_lcd_panel_handle_t lcd_panel = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;

#define LVGL_TICK_PERIOD_MS    5
#define LVGL_TASK_STACK_SIZE   (10 * 1024)
#define LVGL_TASK_PRIORITY     2
#define LVGL_TASK_AFFINITY     -1   
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ

#if LCD_BITS_PER_PIXEL==16
#define LV_COLOR_FORMAT        LV_COLOR_FORMAT_RGB565
#elif LCD_BITS_PER_PIXEL==24
#define LV_COLOR_FORMAT        LV_COLOR_FORMAT_RGB888
#endif

/**
  * @brief  LCD model
  */
 typedef enum
 {
   LCD_XF043AR_WQ  = 0x00, /* XF043AR_WQ */
   LCD_XF05AR_WQ, /* XF05AR_WQ */
   LCD_XF070WV02B_TTUL, /* XF070WV02B_TTUL */

   LCD_TYPE_NUM /* LCD models total */
 }LCD_TypeDef;

const LCD_PARAM_TypeDef lcd_param[LCD_TYPE_NUM]={
    {/* XF043AR_WQ */
        .hbp = 8,
        .vbp = 16,
        .hsw = 4,
        .vsw = 4,
        .hfp = 8,
        .vfp = 16,
        .clock_HZ = 24000000,   //24MHz
        .lcd_pixel_width = 800,
        .lcd_pixel_height = 480,
        .CTP_init = FT5446_Init,
        .CTP_scan = FT5446_ScanV1,
    },
    {/* XF05AR_WQ */
        .hbp = 16,//8,
        .vbp = 32,//16,
        .hsw = 8,//4,
        .vsw = 8,//4,
        .hfp = 16,//8,
        .vfp = 32,//16,
        .clock_HZ = 24000000,   //24MHz
        .lcd_pixel_width = 800,
        .lcd_pixel_height = 480,
        .CTP_init = FT5446_Init,
        .CTP_scan = FT5446_ScanV2,
    },
    {/* XF070WV02B_TTUL */
        .hbp = 40,
        .vbp = 31,
        .hsw = 10,
        .vsw = 6,
        .hfp = 40,
        .vfp = 18,
        .clock_HZ = 30000000,   //30MHz
        .lcd_pixel_width = 800,
        .lcd_pixel_height = 480,
        .CTP_init = GT915_Init,
        .CTP_scan = GT915_Scan,
    },
};

/* Current used LCD, default is LCD_XF05AR_WQ */
// #define CUR_LCD  LCD_XF043AR_WQ
// #define CUR_LCD  LCD_XF05AR_WQ
#define CUR_LCD  LCD_XF070WV02B_TTUL


/**
 * @brief Initialize LCD
 * 
 * @return esp_err_t ESP_OK on success, otherwise error code
 */
static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD backlight */
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_GPIO_BL
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    // /* LCD initialization */
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = LCD_BITS_PER_PIXEL,
        .clk_src = LCD_CLK_SRC_PLL240M,
        .disp_gpio_num = LCD_RGB_DISP,
        .pclk_gpio_num = LCD_RGB_PCLK,
        .vsync_gpio_num = LCD_RGB_VS,
        .hsync_gpio_num = LCD_RGB_HS,
        .de_gpio_num = LCD_RGB_DE,
        .data_gpio_nums = {
            LCD_RGB_D0,
            LCD_RGB_D1,
            LCD_RGB_D2,
            LCD_RGB_D3,
            LCD_RGB_D4,
            LCD_RGB_D5,
            LCD_RGB_D6,
            LCD_RGB_D7,
            LCD_RGB_D8,
            LCD_RGB_D9,
            LCD_RGB_D10,
            LCD_RGB_D11,
            LCD_RGB_D12,
            LCD_RGB_D13,
            LCD_RGB_D14,
            LCD_RGB_D15,
        },
        .timings = {
            .pclk_hz = LCD_CLK_HZ,    //LCD clock frequency
            .h_res = LCD_PIXEL_WIDTH,  //LCD pixel width
            .v_res = LCD_PIXEL_HEIGHT, //LCD pixel height
            .hsync_back_porch = LCD_HBP, //HSYNC back porch
            .hsync_front_porch = LCD_HFP, //HSYNC front porch
            .hsync_pulse_width = LCD_HSW, //HSYNC pulse width
            .vsync_back_porch = LCD_VBP, //VSYNC back porch
            .vsync_front_porch = LCD_VFP, //VSYNC front porch
            .vsync_pulse_width = LCD_VSW, //VSYNC pulse width
        },
        .flags.fb_in_psram = true, // allocate frame buffer from PSRAM
        .flags.double_fb = LCD_DRAW_BUFF_DOUBLE,
        .bounce_buffer_size_px = LCD_PIXEL_WIDTH * LCD_DRAW_BUFF_HEIGHT,
        .num_fbs = LCD_DRAW_BUFF_DOUBLE ? 2 : 1,
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &lcd_panel));


    //hw reset(P1_0)
    aw9523_io_set_level(AW9523_PORT_1, 0, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    aw9523_io_set_level(AW9523_PORT_1, 0, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    aw9523_io_set_level(AW9523_PORT_1, 0, 1);
    vTaskDelay(120 / portTICK_PERIOD_MS);

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_mirror(lcd_panel, true, true);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    return ret;
}

/**
 * @brief Initialize LVGL
 * 
 * @return esp_err_t ESP_OK on success, otherwise error code
 */
static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = LVGL_TASK_PRIORITY,         /* LVGL task priority */
        .task_stack = LVGL_TASK_STACK_SIZE,     /* LVGL task stack size */
        .task_affinity = LVGL_TASK_AFFINITY,        /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = LVGL_TASK_MAX_DELAY_MS,   /* Maximum sleep in LVGL task */
        .timer_period_ms = LVGL_TICK_PERIOD_MS        /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = NULL,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_PIXEL_WIDTH,
        .vres = LCD_PIXEL_HEIGHT,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,//true,
            .mirror_y = false,//true,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = true,
            .swap_bytes = false,
        }
    };

    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = true,
            .avoid_tearing = false,
        }
    };
    lvgl_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);

    /* Add touch input */
    if(CTP_INIT != NULL && CTP_SCAN != NULL)
    {
        CTP_INIT();
        lv_indev_t * indev = lv_indev_create();
        lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev, CTP_SCAN);
    }

    return ESP_OK;
}


/**
 * @brief Demo UI
 * 
 * @return None
 */
void ui_demo(void)
{
    /* Label */
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_obj_set_width(label, LCD_PIXEL_HEIGHT/2);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label, LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"\n "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);

    /* Button */
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    label = lv_label_create(btn);
    lv_label_set_text_static(label, "Rotate screen");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    // lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);

    /* show the screen */
    lv_disp_t * dispp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    lv_disp_load_scr(lv_scr_act());
}

/**
 * @brief Initialize display
 * 
 * @return None
 */
void display_init(void)
{
    app_lcd_init();
    app_lvgl_init();
    
    /* LCD backlight on */
    ESP_ERROR_CHECK(gpio_set_level(LCD_GPIO_BL, LCD_BL_ON_LEVEL));

    /* Task lock */
    lvgl_port_lock(0);

    // ui_demo();
    ui_init();
    
    /* Task unlock */
    lvgl_port_unlock();
}
