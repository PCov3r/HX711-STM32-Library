# HX711-STM32-Library
A simple library to provide control over the hx711 load cell module for STM32

Example functions are provided in the example dir.
According to the datasheet, channel A can implement a gain of 128 or 64, and channel B has a fixed gain of 32.

Each load cell voltage is characterised by the equation:
$U = weight*g+offset$ <br>
The *offset* is removed by the tare function, and *g* is a characteristic of the load cell that needs to be measured.

⚠️ The value of *g* depends on the load cell and should not be confused with the HX711 channel gain.⚠️

## Load cell gain

The load cell gain can be easily measured. The procedure is as follow:
* First, initialize the module and set the scaling factor to 1 (``set_scale()``).
* Then, tare the scale (``tare_all()``).
* Once the module has been correctly initialized, measure a known weight and note the value obtained from the HX711 module.
* Divide this value by the real value. The result is the parameter you need to pass to set_scale.
