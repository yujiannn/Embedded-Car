#ifndef ENCODER_H
#define ENCODER_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define LEFT_ENCODER_PIN 16
#define RIGHT_ENCODER_PIN 17
#define WHEEL_CIRCUMFERENCE (204.1f / 100) // Wheel circumference in meters

void gpio_callback(uint gpio, uint32_t events);
void reset_encoders();
void init_encoders();

#endif // ENCODER_H
