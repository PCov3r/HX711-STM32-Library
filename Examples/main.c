/*
 * This file contains example functions to be implemented in your main.c file.
 * The init_weight initialized the HX711 module with the desired gain and is used
 * for the tare weight. The function measure_weight can then be used to obtain
 * the weight measured by each load cell (Channel A and B). 
 *
 *  Created on: 28 may 2023
 *      Author: PCov3r
 */

#include <HX711.h>

/**
 * @brief  Weight Initialization Function
 * @retval None
 */
void init_weight(hx711_t *hx711){
	char buffer[128] = {0};

	sprintf(buffer,"HX711 initialization\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t *)(buffer), sizeof(buffer), 100);

	/* Initialize the hx711 sensors */
	hx711_init(hx711, GPIOC, GPIO_PIN_12, GPIOC, GPIO_PIN_10);
  
  /* Configure gain for each channel (see datasheet for details) */
	set_gain(hx711, 128, 32);

  /* Set HX711 scaling factor (see README for procedure) */
	set_scale(hx711, -44.25, -10.98);

  /* Tare weight */
	tare_all(hx711, 10);

	sprintf(buffer,"HX711 module has been initialized\n\r");
	HAL_UART_Transmit(&huart2, (uint8_t *)(buffer), sizeof(buffer), 100);
}

/**
 * @brief  Weight Measuring Function
 * @retval the weight measured for each associated channel
 */

float measure_weight(hx711_t hx711, hx711_t hx711b){
	long weightA = 0;
  long weightB = 0;

  // Measure the weight for channel A
    weightA = get_weight(&hx711, 10, CHANNEL_A);
  // Weight cannot be negative
    weightA = (weightA < 0) ? 0 : weightA;

  // Measure the weight for channel B
    weightB = get_weight(&hx711, 10, CHANNEL_B);
  // Weight cannot be negative
    weightB = (weightB < 0) ? 0 : weightB;
  
    return weightA;
}


