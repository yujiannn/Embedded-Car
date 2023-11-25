#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "pico/stdlib.h"
#include "hardware/adc.h"

#define IR_SENSOR_LEFT_GPIO_PIN 27
#define IR_SENSOR_RIGHT_GPIO_PIN 28

// Define the ADC channels corresponding to the IR sensor GPIO pins
#define LEFT_IR_SENSOR_ADC_CHANNEL 1  // ADC Channel for GPIO 27
#define RIGHT_IR_SENSOR_ADC_CHANNEL 2 // ADC Channel for GPIO 28

void initIRSensorsADC();
uint16_t readFromADCChannel(uint channel);
int checkLineSensors();
void debugIRSensors();

#endif // IR_SENSOR_H
