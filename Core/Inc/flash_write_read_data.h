#ifndef INC_FLASH_WRITE_READ_DATA_H_
#define INC_FLASH_WRITE_READ_DATA_H_

#include <stdint.h>
void flash_data_read (uint32_t address, uint32_t *pWData, uint16_t words_lenght);
uint32_t flash_data_write (uint32_t address, uint32_t *pWData, uint16_t words_lenght);

#endif /* INC_FLASH_WRITE_READ_DATA_H_ */
