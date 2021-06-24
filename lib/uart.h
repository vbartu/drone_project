#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <stdbool.h>

bool uart_data_ready(void);
uint8_t uart_read(void);

#endif // __UART_H
