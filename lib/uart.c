#include <machine/patmos.h>

#include "uart.h"

#define TRANSMIT_READY_MASK 0x00000001U
#define DATA_AVAILABLE_MASK 0x00000002U
#define UART2_CTRL_REGISTER *(int*) (PATMOS_IO_UART2)
#define UART2_DATA_REGISTER *(int*) (PATMOS_IO_UART2 + 4)
#define UART3_CTRL_REGISTER *(int*) (PATMOS_IO_UART3)
#define UART3_DATA_REGISTER *(int*) (PATMOS_IO_UART3 + 4)


bool uart_data_ready(void)
{
	return UART2_CTRL_REGISTER & DATA_AVAILABLE_MASK;
}

uint8_t uart_read(void)
{
	return UART2_DATA_REGISTER;
}
