#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Function declarations
void setupUltrasonicPins(uint trigPin, uint echoPin);
void echo_pin_isr(uint gpio, uint32_t events);
uint64_t getPulse(uint trigPin, uint echoPin);
double getCm(uint trigPin, uint echoPin);
void ultrasonicSensorRead(uint trigPin, uint echoPin);

#endif // ULTRASONIC_SENSOR_H
