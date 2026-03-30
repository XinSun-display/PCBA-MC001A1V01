#include <stdio.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "display.h"
#include "aw9523.h"
#include "BSP_TWAI.h"

static const char *TAG = "MAIN";

void log_ram_usage(void) 
{
    // Check internal SRAM 
    size_t internal_free = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t internal_total = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    
    // Check external PSRAM 
    size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    ESP_LOGI(TAG, "Memory Stats:");
    ESP_LOGI(TAG, "  Internal SRAM: Free: %d KB / Total: %d KB", internal_free / 1024, internal_total / 1024);
    ESP_LOGI(TAG, "  External PSRAM: Free: %d KB / Total: %d KB", psram_free / 1024, psram_total / 1024);
    
    // Check largest free block of PSRAM (this can show the memory fragmentation)
    ESP_LOGI(TAG, "  Max Allocatable PSRAM Block: %d KB", 
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM) / 1024);
}

void app_main(void)
{
    /* AW9523 INIT*/
    aw9523_init(48,47);
    /* Set AW9523_PORT0 Push-pull output*/
    aw9523_set_port0_pp(1);
    /* Set AW9523_PORT0 and AW9523_PORT1 level*/
    aw9523_set_level(AW9523_PORT_0, 0);
    aw9523_set_level(AW9523_PORT_1, 0);

    display_init();

    twai_init();

    // log_ram_usage();

    while(1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}