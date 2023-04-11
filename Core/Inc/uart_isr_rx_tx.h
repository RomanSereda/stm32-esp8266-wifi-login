#ifndef _UART_ISR_RX_TX_H_
#define _UART_ISR_RX_TX_H_

#include "stm32f4xx_hal.h"

/* change the size of the buffer */
#define UART_BUFFER_SIZE 96
#define UART_TIMEOUT_RET -1

typedef struct
{
  unsigned char buffer[UART_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
} ring_buffer;

/* Initialize the ring buffer */
void uart_isr_init(void);

/* reads the data in the rx_buffer and increment the tail count in rx_buffer */
int uart_isr_read(void);

/* writes the data to the tx_buffer and increment the head count in tx_buffer */
void uart_isr_write(int c);

/* function to send the string to the uart */
void uart_isr_sendstring(const char *s);

/* Print a number with any base
 * base can be 10, 8 etc*/
void uart_isr_printbase (long n, uint8_t base);

/* Checks if the data is available to read in the rx_buffer */
int uart_isr_isDataAvailable(void);

/* Look for a particular string in the given buffer
 * @return 1, if the string is found and -1 if not found
 * @USAGE:: if (Look_for ("some string", buffer)) do something
 */
int uart_isr_look_for (char *str, char *buffertolookinto);

/* Copies the required data from a buffer
 * @startString: the string after which the data need to be copied
 * @endString: the string before which the data need to be copied
 * @USAGE:: GetDataFromBuffer ("name=", "&", buffertocopyfrom, buffertocopyinto);
 */
void uart_isr_getDataFromBuffer (char *startString, char *endString, char *buffertocopyfrom, char *buffertocopyinto);


/* Resets the entire ring buffer, the new data will start from position 0 */
void uart_isr_flush (void);

/* Peek for the data in the Rx Bffer without incrementing the tail count
* Returns the character
* USAGE: if (Uart_peek () == 'M') do something
*/
int uart_isr_peek();

/* Copy the data from the Rx buffer into the bufferr, Upto and including the entered string
* This copying will take place in the blocking mode, so you won't be able to perform any other operations
* Returns 1 on success and -1 otherwise
* USAGE: while (!(Copy_Upto ("some string", buffer)));
*/
int uart_isr_copy_upto (char *string, char *buffertocopyinto);

/* Copies the entered number of characters (blocking mode) from the Rx buffer into the buffer, after some particular string is detected
* Returns 1 on success and -1 otherwise
* USAGE: while (!(Get_after ("some string", 6, buffer)));
*/
int uart_isr_get_after (char *string, uint8_t numberofchars, char *buffertosave);

/* Wait until a paricular string is detected in the Rx Buffer
* Return 1 on success and -1 otherwise
* USAGE: while (!(Wait_for("some string")));
*/
int uart_isr_wait_for (char *string);

/* the ISR for the uart. put it in the IRQ handler */
void uart_isr (UART_HandleTypeDef *huart);

#endif
