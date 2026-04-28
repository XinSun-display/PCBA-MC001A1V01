#ifndef __BSP_TWAI_H__
#define __BSP_TWAI_H__

#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

void twai_init(void);
esp_err_t twai_send(twai_message_t *message);
QueueHandle_t* twai_get_rx_queue(void);

#ifdef __cplusplus
}
#endif

#endif