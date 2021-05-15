/*

  ******************************************************************************
  * @file 			( фаил ):   AT24Cxx.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
*/

/* Includes ----------------------------------------------------------*/
#include "AT24Cxx.h"

uint8_t data_buff[ AT24CXX_PAGE_BYTE ];

/*
	******************************************************************************
	* @brief	 ( описание ):  Отчиска всего чипа ( 0xFF )
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
void AT24Cxx_erase_chip( void ){
	
	#if defined (WP_GPIO_Port)
		HAL_GPIO_WritePin ( WP_GPIO_Port, WP_Pin, GPIO_PIN_RESET );	// отключаем защиту от записи
	#endif
	
	memset( data_buff, 0xFF, AT24CXX_PAGE_BYTE );
	
	for( uint16_t i = 0; i < AT24CXX_MAX_MEM_ADDRESS; i = i + AT24CXX_PAGE_BYTE ){
			
		HAL_I2C_Mem_Write( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, i, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_buff, AT24CXX_PAGE_BYTE, HAL_MAX_DELAY );
		
		HAL_Delay ( 10 );
	}
	
	#if defined (WP_GPIO_Port)
		HAL_GPIO_WritePin ( WP_GPIO_Port, WP_Pin, GPIO_PIN_SET );	// включаем защиту от записи
	#endif
	
}
//----------------------------------------------------------------------------------

/*
	******************************************************************************
	* @brief	 ( описание ):  Функция для записи в память массив данных ( uint8_t )
	* @param	( параметры ):	1- адрес в памяти значение от 0 до AT24CXX_MAX_MEM_ADDRESS
								2- сам массив с данными
								3- размер массива ( или части )в байтах, сколько хотим записать байт 
	* @return  ( возвращает ):	вернет 0 если запись не произошла ( передали адрес и размер массива больше чем память чипа )
								вернет ( если запись удалась ) адресс на котором зокончили запись, удобно для последуещей записи с конца.

	******************************************************************************
*/
uint16_t AT24Cxx_write( uint16_t addMem_write, uint8_t *data_write, uint16_t size_write){
	
	if( (addMem_write + size_write) < AT24CXX_MAX_MEM_ADDRESS ){
		
		#if defined (WP_GPIO_Port)
			HAL_GPIO_WritePin ( WP_GPIO_Port, WP_Pin, GPIO_PIN_RESET );	// отключаем защиту от записи
		#endif
		
		uint16_t page_count_write = addMem_write / AT24CXX_PAGE_BYTE;	// узнаем на какой странице мы находимся
		uint16_t byte_count_write = AT24CXX_PAGE_BYTE - ( addMem_write - (page_count_write * AT24CXX_PAGE_BYTE));	// узнаем сколько байт нужно отправить до следуещей страницы
			
		if( byte_count_write >= size_write ){	// если размер данных помещается в остаток до конца страницы
			
			memcpy( data_buff, data_write, size_write );
		
			HAL_I2C_Mem_Write( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, addMem_write, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_buff, size_write, HAL_MAX_DELAY );
				
			HAL_Delay ( 10 );
		}
		else{	// если размер данных не помещается в остаток до конца страницы
			
			memcpy( data_buff, data_write, byte_count_write );
		
			HAL_I2C_Mem_Write( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, addMem_write, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_buff, byte_count_write, HAL_MAX_DELAY );
				
			HAL_Delay ( 10 );
			
			size_write = size_write - byte_count_write;
			addMem_write = addMem_write + byte_count_write;
			uint16_t data_offset_write = byte_count_write;
			
			// если остаток не помещается до конца страницы ( размер на больше чем страница
			while( size_write >= AT24CXX_PAGE_BYTE ){
				
				memcpy( data_buff, data_write + data_offset_write, AT24CXX_PAGE_BYTE );
		
				HAL_I2C_Mem_Write( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, addMem_write, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_buff, AT24CXX_PAGE_BYTE, HAL_MAX_DELAY );
				
				HAL_Delay ( 10 );
				
				size_write = size_write - AT24CXX_PAGE_BYTE;
				addMem_write = addMem_write + AT24CXX_PAGE_BYTE;
				data_offset_write = data_offset_write + AT24CXX_PAGE_BYTE;
				
			}
			if( size_write ){
			
				memcpy( data_buff, data_write + data_offset_write, size_write );
			
				HAL_I2C_Mem_Write( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, addMem_write, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_buff, size_write, HAL_MAX_DELAY );
					
				HAL_Delay ( 10 );
			}
			
		}
		
		#if defined (WP_GPIO_Port)
			HAL_GPIO_WritePin ( WP_GPIO_Port, WP_Pin, GPIO_PIN_SET );	// включаем защиту от записи
		#endif
		
		return addMem_write + size_write;
	}
	else{
		return 0;
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  Функция для чтения из памяти в массив данных ( uint8_t )
	* @param	( параметры ):	1- адрес в памяти значение от 0 до AT24CXX_MAX_MEM_ADDRESS
								2- сам массив для данных
								3- размер массива ( или части )в байтах, сколько хотим считать байт 
	* @return  ( возвращает ):	вернет 0 если чтение не произошло ( передали адрес и размер массива больше чем память чипа )
								вернет ( если чтение удалось ) адресс на котором зокончили чтение, удобно для последуещего чтения с конца.

	******************************************************************************
*/
uint16_t AT24Cxx_read( uint16_t addMem_read, uint8_t *data_read, uint16_t size_read){
	
	if( (addMem_read + size_read) < AT24CXX_MAX_MEM_ADDRESS ){
				
		HAL_I2C_Mem_Read( &AT24CXX_I2C,  AT24CXX_I2C_ADDR, addMem_read, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data_read, size_read, HAL_MAX_DELAY );
		
		return addMem_read + size_read;
	}
	else{
		return 0;
	}
}
//----------------------------------------------------------------------------------------


/************************ (C) COPYRIGHT GKP *****END OF FILE****/
