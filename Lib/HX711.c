/*
 * HX711.c
 *
 *  Created on: 16 nov. 2021
 *      Author: PCov3r
 */

#include <HX711.h>

//#############################################################################################
void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *dat_gpio, uint16_t dat_pin){
  // Setup the pin connections with the STM Board
  hx711->clk_gpio = clk_gpio;
  hx711->clk_pin = clk_pin;
  hx711->dat_gpio = dat_gpio;
  hx711->dat_pin = dat_pin;

  GPIO_InitTypeDef  gpio = {0};
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pin = clk_pin;
  HAL_GPIO_Init(clk_gpio, &gpio);
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pin = dat_pin;
  HAL_GPIO_Init(dat_gpio, &gpio);

}

//#############################################################################################
void set_scale(hx711_t *hx711, float Ascale, float Bscale){
  // Set the scale. To calibrate the cell, run the program with a scale of 1, call the tare function and then the get_units function. 
  // Divide the obtained weight by the real weight. The result is the parameter to pass to scale
	hx711->Ascale = Ascale;
	hx711->Bscale = Bscale;
}

//#############################################################################################
void set_gain(hx711_t *hx711, uint8_t Again, uint8_t Bgain){
  // Define A channel's gain
	switch (Again) {
			case 128:		// channel A, gain factor 128
				hx711->Again = 1;
				break;
			case 64:		// channel A, gain factor 64
				hx711->Again = 3;
				break;
		}
	hx711->Bgain = 2;
}

//#############################################################################################
void set_offset(hx711_t *hx711, long offset, uint8_t channel){
	if(channel == CHANNEL_A) hx711->Aoffset = offset;
	else hx711->Boffset = offset;
}

//############################################################################################
uint8_t shiftIn(hx711_t *hx711, uint8_t bitOrder) {
    uint8_t value = 0;
    uint8_t i;

    for(i = 0; i < 8; ++i) {
    	HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, SET);
        if(bitOrder == 0)
            value |= HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) << i;
        else
            value |= HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) << (7 - i);
        HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, RESET);
    }
    return value;
}

//############################################################################################
bool is_ready(hx711_t *hx711) {
	if(HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) == GPIO_PIN_RESET){
		return 1;
	}
	return 0;
}

//############################################################################################
void wait_ready(hx711_t *hx711) {
	// Wait for the chip to become ready.
	while (!is_ready(hx711)) {
		HAL_Delay(0);
	}
}

//############################################################################################
long read(hx711_t *hx711, uint8_t channel){
	wait_ready(hx711);
	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	noInterrupts();

	data[2] = shiftIn(hx711, 1);
	data[1] = shiftIn(hx711, 1);
	data[0] = shiftIn(hx711, 1);

	uint8_t gain = 0;
	if(channel == 0) gain = hx711->Again;
	else gain = hx711->Bgain;

	for (unsigned int i = 0; i < gain; i++) {
		HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, SET);
		HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, RESET);
	}

	interrupts();

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( (unsigned long)(filler) << 24
			| (unsigned long)(data[2]) << 16
			| (unsigned long)(data[1]) << 8
			| (unsigned long)(data[0]) );

	return (long)(value);
}

//############################################################################################
long read_average(hx711_t *hx711, int8_t times, uint8_t channel) {
	long sum = 0;
	for (int8_t i = 0; i < times; i++) {
		sum += read(hx711, channel);
		HAL_Delay(0);
	}
	return sum / times;
}

//############################################################################################
double get_value(hx711_t *hx711, int8_t times, uint8_t channel) {
	long offset = 0;
	if(channel == CHANNEL_A) offset = hx711->Aoffset;
	else offset = hx711->Boffset;
	return read_average(hx711, times, channel) - offset;
}

//############################################################################################
void tare(hx711_t *hx711, uint8_t times, uint8_t channel) {
	read(hx711, channel); // Change channel
	double sum = read_average(hx711, times, channel);
	set_offset(hx711, sum, channel);
}

//############################################################################################
void tare_all(hx711_t *hx711, uint8_t times) {
	tare(hx711, times, CHANNEL_A);
	tare(hx711, times, CHANNEL_B);
}

//############################################################################################
float get_weight(hx711_t *hx711, int8_t times, uint8_t channel) {
  // Read load cell
	read(hx711, channel);
	float scale = 0;
	if(channel == CHANNEL_A) scale = hx711->Ascale;
	else scale = hx711->Bscale;
	return get_value(hx711, times, channel) / scale;
}
