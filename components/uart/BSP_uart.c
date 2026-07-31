#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "BSP_uart.h"
#include "aw9523.h" 

static const char *TAG = "UART";

#define UART_QUEUE_LENGTH 10
QueueHandle_t uart_TX_message_queue;
QueueHandle_t uart_RX_message_queue;

#define EXT_UART_NUM        UART_NUM_0
#define EXT_UART_BAUD       9600//115200
#define EXT_UART_TX_GPIO    43
#define EXT_UART_RX_GPIO    44
QueueHandle_t uart_queue;
const int uart_buffer_size = 1024;

/**
 * @brief Send task
 */
void uart_send_task(void *arg)
{
    ESP_LOGI(TAG, "UART send task started");
    while (1)
    {
        UART_Msg_t message;
        if (xQueueReceive(uart_TX_message_queue, &message, portMAX_DELAY) == pdPASS)
        {
            aw9523_io_set_level(AW9523_PORT_1, 3, 1); // Set RS485 to send mode
            uart_write_bytes(EXT_UART_NUM, message.data, message.length);
            uart_wait_tx_done(EXT_UART_NUM, portMAX_DELAY);
            aw9523_io_set_level(AW9523_PORT_1, 3, 0); // Set RS485 to receive mode
        }
    }
}

/**
 * @brief UART read task
 * 
 * @param pvParameter
 * @return void
 */
void uart_read_task(void *pvParameter)
{
    uart_event_t event;
    uint8_t dtmp[1024]; 

    ESP_LOGI(TAG, "UART read task started");

    aw9523_io_set_level(AW9523_PORT_1, 3, 0); // Set RS485 to receive mode
    while (1)
    {
        // 1. Wait for UART event using event queue 
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) 
        {
            bzero(dtmp, sizeof(dtmp));
            
            switch (event.type) 
            {
                // 2. When notified that data is received, read it all at once
                case UART_DATA:            
                    // Read all data indicated by the event length
                    int len = uart_read_bytes(EXT_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    if (len > 0) {
                        // Also print the received data for debugging
                        ESP_LOGI(TAG, "Received %d bytes: %s", len, dtmp);

                        UART_Msg_t uart_RX_data;
                        size_t copy_len = len;
                        if (copy_len > sizeof(uart_RX_data.data)) {
                            ESP_LOGW(TAG, "UART frame too long (%d), truncate to %d", len, sizeof(uart_RX_data.data));
                            copy_len = sizeof(uart_RX_data.data);
                        }
                        uart_RX_data.length = copy_len;
                        memcpy(uart_RX_data.data, dtmp, copy_len);
                        if (xQueueSend(uart_RX_message_queue, &uart_RX_data, pdMS_TO_TICKS(10)) != pdPASS) {
                            ESP_LOGW(TAG, "uart_RX_message_queue full, drop frame");
                        }
                    }
                    break;
                    
                // Handle other exceptional events to prevent UART from getting stuck
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "hw fifo overflow");
                    uart_flush_input(EXT_UART_NUM);
                    xQueueReset(uart_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "ring buffer full");
                    uart_flush_input(EXT_UART_NUM);
                    xQueueReset(uart_queue);
                    break;
                default:
                    break;
            }
        }
    }
}

/**
 * @brief UART initialization. UART & RS485 & RS232 share the same interface. Please use jumper (J10, J11, J12) to select one output.
 * @param void
 * @return void
 */
void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = EXT_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver and enable event queue
    ESP_ERROR_CHECK(uart_driver_install(EXT_UART_NUM, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(EXT_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(EXT_UART_NUM, EXT_UART_TX_GPIO, EXT_UART_RX_GPIO,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART0 ready (TX=%d RX=%d @ %d)",
             EXT_UART_TX_GPIO, EXT_UART_RX_GPIO, EXT_UART_BAUD);

    aw9523_io_set_level(AW9523_PORT_1, 3, 1); // Set RS485 to send mode
    uart_write_bytes(EXT_UART_NUM, "Hello\r\n", 7);
    uart_wait_tx_done(EXT_UART_NUM, 10 / portTICK_PERIOD_MS);

    uart_TX_message_queue = xQueueCreate(UART_QUEUE_LENGTH, sizeof(UART_Msg_t));
    uart_RX_message_queue = xQueueCreate(UART_QUEUE_LENGTH, sizeof(UART_Msg_t));

    xTaskCreate(uart_read_task, "uart_read_task", 4096, NULL, 5, NULL);
    xTaskCreate(uart_send_task, "uart_send_task", 4096, NULL, 5, NULL);
}

/**
 * @brief Send a message
 * @param message Pointer to the message to send
 * @return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t uart_send(UART_Msg_t *message)
{
    if (xQueueSend(uart_TX_message_queue, message, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "uart_TX_message_queue full, drop frame");
        return ESP_FAIL;
    }
    return ESP_OK;
}

/**
 * @brief Get the RX queue handle
 * @return QueueHandle_t*
 */
QueueHandle_t* uart_get_rx_queue(void)
{
    return &uart_RX_message_queue;
}
