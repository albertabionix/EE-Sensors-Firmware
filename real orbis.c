#include "orbis.h"

static uint8_t crc8_0x97(const uint8_t *data, int len) // from dylan
{
    uint8_t crc = 0x00;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++)
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x97) : (uint8_t)(crc << 1);
    }
    return crc;
}


HAL_StatusTypeDef Orbis_Read(struct Orbis *enc)
{
    if (enc == NULL || enc->hspi == NULL)
    {
        return HAL_ERROR;
    }
    // 3 bytes used based off encoder data sheet with 24 bits/8 bits

    uint8_t tx_data[3] = {0}; // transmits 3 bytes of zeros to read the position
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

    if (status != HAL_OK)
    {
        return status; // return if the SPI transfer failed
    }

    if ((uint8_t)~crc8_0x97(rx_data, 2) != rx_data[2]) // from dylan
{
    return HAL_ERROR;
}
    
    uint16_t data_position = ((uint16_t)rx_data[0] << 8) | rx_data[1]; // combines the first two bytes to get the 14-bit position
    // by keeping the lower 6 bits of the first byte and shifting it into upper 8 bits and combines the second byte as lower 8 bits
    enc->position = (data_position >> 2) & 0x3FFF; // updates the position in the struct by shifting data 2 positions and keeps lowest 14 bits
    enc->error = !(data_position & 0x02); // checks if the error bit is set 
    enc->warning = !(data_position & 0x01); // checks if the warning bit is set 
// position in b15 to b2, b1 is error, and b- is warning

    if (enc->error)
    {
        return HAL_ERROR;
    }

    return HAL_OK; // return if the SPI transfer works
}
