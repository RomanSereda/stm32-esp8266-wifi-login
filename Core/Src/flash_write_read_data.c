#include "flash_write_read_data.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"

static uint32_t get_sector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < 0x08003FFF) && (Address >= 0x08000000))
    {
      sector = FLASH_SECTOR_0;
    }
    else if((Address < 0x08007FFF) && (Address >= 0x08004000))
    {
      sector = FLASH_SECTOR_1;
    }
    else if((Address < 0x0800BFFF) && (Address >= 0x08008000))
    {
      sector = FLASH_SECTOR_2;
    }
    else if((Address < 0x0800FFFF) && (Address >= 0x0800C000))
    {
      sector = FLASH_SECTOR_3;
    }
    else if((Address < 0x0801FFFF) && (Address >= 0x08010000))
    {
      sector = FLASH_SECTOR_4;
    }
    else if((Address < 0x0803FFFF) && (Address >= 0x08020000))
    {
      sector = FLASH_SECTOR_5;
    }
  return sector;
}

uint32_t flash_data_write (uint32_t address, uint32_t *pWData, uint16_t words_lenght)
{
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SECTOR_Error;
	int index = 0;

	HAL_FLASH_Unlock();

	uint32_t start_sector = get_sector(address);
	uint32_t end_sector_address = address + words_lenght * 4;
	uint32_t end_sector = get_sector(end_sector_address);

	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = start_sector;
	EraseInitStruct.NbSectors     = (end_sector - start_sector) + 1;

	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SECTOR_Error) != HAL_OK)
		return HAL_FLASH_GetError ();


	while (index < words_lenght){
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, pWData[index]) == HAL_OK) address += 4;
	    else return HAL_FLASH_GetError();
		index++;
	}

	HAL_FLASH_Lock();
	return 0;
}

void flash_data_read (uint32_t address, uint32_t *pWData, uint16_t words_lenght)
{
	for(int i = 0; i < words_lenght; ++i)
	{
		pWData[i] = *(__IO uint32_t*)address;
		address += 4;
	}
}
