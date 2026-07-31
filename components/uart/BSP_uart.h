#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t length;        
    char data[64];         
} UART_Msg_t;

void uart_init(void);
QueueHandle_t* uart_get_rx_queue(void);
esp_err_t uart_send(UART_Msg_t *message);

#ifdef __cplusplus
}
#endif

#endif