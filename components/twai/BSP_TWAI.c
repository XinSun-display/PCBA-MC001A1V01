#include "driver/twai.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "BSP_TWAI.h"

#define TAG "TWAI"

twai_TX_message_t twai_TX_message[TWAI_MESSAGE_POOL_SIZE]={0};
twai_RX_message_pool_t twai_RX_message_pool={0};

void twai_send_task(void *arg) 
{
    while (1) {
        for (int i = 0; i < TWAI_MESSAGE_POOL_SIZE; i++) {
            if (twai_TX_message[i].state == TWAI_TX_STATE_READY) {
                twai_TX_message[i].state = TWAI_TX_STATE_SENDING;
                esp_err_t err = twai_transmit(&twai_TX_message[i].message, pdMS_TO_TICKS(1000));
                if (err == ESP_OK) {
                    twai_TX_message[i].state = TWAI_TX_STATE_IDLE;
                    // ESP_LOGI(TAG, "Message sent successfully");
                } else {
                    ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
                    twai_TX_message[i].state = TWAI_TX_STATE_IDLE;
                }
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void twai_receive_task(void *arg) 
{
    while (1) {
        twai_message_t message;
        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(1000));
        if (err == ESP_OK) 
        {
            ESP_LOGI(TAG, "Message received: ID=0x%X, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                     message.identifier,
                     message.data[0], message.data[1], message.data[2], message.data[3],
                     message.data[4], message.data[5], message.data[6], message.data[7]);

            memcpy(&twai_RX_message_pool.message[twai_RX_message_pool.in_index], &message, sizeof(twai_message_t));
            twai_RX_message_pool.is_received[twai_RX_message_pool.in_index] = true;
            twai_RX_message_pool.in_index=(twai_RX_message_pool.in_index+1)%TWAI_MESSAGE_POOL_SIZE;
        } else if (err == ESP_ERR_TIMEOUT) 
        {
            // ESP_LOGW(TAG, "Reception timed out");
        } else 
        {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
        }
    }
}

void twai_init(void)
{
    // Configure TWAI driver
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_2, GPIO_NUM_1, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS();  // Set to 100 Kbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all messages

    // Install TWAI driver
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Start TWAI driver
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    // Create tasks
    xTaskCreate(twai_send_task, "twai_send_task", 2048, NULL, 5, NULL);
    xTaskCreate(twai_receive_task, "twai_receive_task", 2048, NULL, 5, NULL);
}

esp_err_t twai_send(twai_message_t *message)
{
    for (int i = 0; i < TWAI_MESSAGE_POOL_SIZE; i++) {
        if (twai_TX_message[i].state == TWAI_TX_STATE_IDLE) {
            memcpy(&twai_TX_message[i].message, message, sizeof(twai_message_t));
            twai_TX_message[i].state = TWAI_TX_STATE_READY;
            return ESP_OK;
        }
    }
    ESP_LOGE(TAG, "TWAI TX message pool is full");
    return ESP_ERR_NO_MEM;
}

esp_err_t twai_receive_get(twai_message_t *message)
{
    for (int i = 0; i < TWAI_MESSAGE_POOL_SIZE; i++) {
        if (twai_RX_message_pool.is_received[i]) {
            memcpy(message, &twai_RX_message_pool.message[i], sizeof(twai_message_t));
            twai_RX_message_pool.is_received[i] = false;
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}