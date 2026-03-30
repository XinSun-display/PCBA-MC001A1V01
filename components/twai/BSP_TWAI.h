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

#define TWAI_MESSAGE_POOL_SIZE (10)

typedef enum twai_TX_state_t
{
    TWAI_TX_STATE_IDLE,
    TWAI_TX_STATE_READY,
    TWAI_TX_STATE_SENDING,
} twai_TX_state_t;

typedef struct twai_TX_message_t
{
    volatile twai_TX_state_t state;
    twai_message_t message;
} twai_TX_message_t;
extern twai_TX_message_t twai_TX_message[TWAI_MESSAGE_POOL_SIZE];

typedef struct twai_RX_message_pool_t
{
    int in_index;
    twai_message_t message[TWAI_MESSAGE_POOL_SIZE];
    volatile bool is_received[TWAI_MESSAGE_POOL_SIZE];
} twai_RX_message_pool_t;
extern twai_RX_message_pool_t twai_RX_message_pool;

void twai_init(void);
esp_err_t twai_TX_send(twai_message_t *message);
esp_err_t twai_receive_get(twai_message_t *message);

#ifdef __cplusplus
}
#endif

#endif