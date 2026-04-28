#include "driver/twai.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "BSP_TWAI.h"

#define TAG "TWAI"
#define TWAI_QUEUE_LENGTH 10
QueueHandle_t twai_TX_message_queue;
QueueHandle_t twai_RX_message_queue;

/**
 * @brief Send task
 */
void twai_send_task(void *arg) 
{
    ESP_LOGI(TAG, "TWAI send task started");
    while (1) {
        twai_message_t message;
        if(xQueueReceive(twai_TX_message_queue, &message, portMAX_DELAY) == pdPASS)
        {
            esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(1000));

            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Message sent successfully");
            } else {
                ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
            }
        }
    }
}

/**
 * @brief Receive task
 */
void twai_receive_task(void *arg) 
{
    ESP_LOGI(TAG, "TWAI receive task started");
    while (1) {
        twai_message_t message;
        esp_err_t err = twai_receive(&message, portMAX_DELAY);
        if (err == ESP_OK) 
        {
            if (uxQueueMessagesWaiting(twai_RX_message_queue) == TWAI_QUEUE_LENGTH) {
                // First, remove the oldest entry by reading it out and discarding it
                twai_message_t message_temp;
                xQueueReceive(twai_RX_message_queue, &message_temp, 0); 
            }
            // There is now space available, so the new data can be inserted
            xQueueSend(twai_RX_message_queue, &message, 0);
        } else if (err == ESP_ERR_TIMEOUT) 
        {
            ESP_LOGW(TAG, "Reception timed out");
        } else 
        {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
        }
    }
}

/**
 * @brief Initialize the TWAI driver
 */
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

    twai_TX_message_queue = xQueueCreate(TWAI_QUEUE_LENGTH, sizeof(twai_message_t));
    twai_RX_message_queue = xQueueCreate(TWAI_QUEUE_LENGTH, sizeof(twai_message_t));
    // Create tasks
    xTaskCreate(twai_send_task, "twai_send_task", 3072, NULL, 5, NULL);
    xTaskCreate(twai_receive_task, "twai_receive_task", 3072, NULL, 5, NULL);
}

/**
 * @brief Send a message
 * @param message Pointer to the message to send
 * @return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t twai_send(twai_message_t *message)
{
    esp_err_t ret = ESP_OK;

    if(xQueueSend(twai_TX_message_queue, message, pdMS_TO_TICKS(100)) == pdPASS)
    {
        ret = ESP_OK;
    }
    else
    {
        ret = ESP_FAIL;
    }

    return ret;
}

/**
 * @brief Get the RX queue handle
 * @return QueueHandle_t*
 */
QueueHandle_t* twai_get_rx_queue(void)
{
    return &twai_RX_message_queue;
}

//******************* example *******************
void example_twai(void)
{
    twai_message_t message;

    //init
    twai_init();

    //send
    ESP_LOGI(TAG, "Sending data...");
    message.identifier = 0x000;
    message.data_length_code = 8;
    message.data[0] = 0x0A;
    message.data[1] = 0x0B;
    message.data[2] = 0x0C;
    message.data[3] = 0x0D;
    message.data[4] = 0x0E;
    message.data[5] = 0x0F;
    message.data[6] = 0x10;
    message.data[7] = 0x11;
    message.extd = 0;                // Standard 11-bit identifier
    message.rtr = 0;                 // Data frame (not a remote frame)
    message.ss = 1;                  // Not single shot
    message.self = 0;                // Normal transmission (not self reception)
    esp_err_t err = twai_send(&message);
    if(err == ESP_OK)
    {
        ESP_LOGI(TAG, "Message sent: %s ID=0x%03lX, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]\r\n",
            message.extd ? "Extended" : "Standard",
            message.identifier,
            message.data[0], message.data[1], message.data[2], message.data[3],
            message.data[4], message.data[5], message.data[6], message.data[7]);
    }
    else
    {
        ESP_LOGE(TAG, "Message send failed: %s", esp_err_to_name(err));
    }

    //receive 
    ESP_LOGI(TAG, "waiting for receiving data...");
    QueueHandle_t* twai_RX_message_queue = twai_get_rx_queue();
    if(xQueueReceive(*twai_RX_message_queue, &message, pdMS_TO_TICKS(100)) == pdPASS)//WAIT
    {                   
        ESP_LOGI(TAG, "Message received: %s ID=0x%03lX, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]\r\n",
                message.extd ? "Extended" : "Standard",
                message.identifier,
                message.data[0], message.data[1], message.data[2], message.data[3],
                message.data[4], message.data[5], message.data[6], message.data[7]);
    }
}
