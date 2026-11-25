#include "orbis.h"

/* Placeholder until the real SPI read is written. Always fails, so main.c never uses a fake position. */
HAL_StatusTypeDef Orbis_Read(struct Orbis *enc)
{
    if (enc == NULL || enc->hspi == NULL)
    {
        return HAL_ERROR;
    }
    // 3 bytes used based off encoder data sheet with 24 bits/8 bits

    uint8_t tx_data[3] = {0x00, 0x00, 0x00}; // transmits 3 bytes of zeros to read the position
    uint8_t rx_data[3] = {0}; // receives 3 bytes of data 
    
    HAL_GPIO_WritePin(enc->cs_port, enc->cs_pin, GPIO_PIN_RESET); // selects the encoder

    HAL_Delay(1); // 1 ms delay

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
        enc->hspi, // this uses the spi handle from the orbis struct instead of hardcoding hspi1
        tx_data,
        rx_data,
        3,
        100
    );
    
    HAL_GPIO_WritePin(enc->cs_port, enc->cs_pin, GPIO_PIN_SET); // deselects the encoder
    return status;
}
