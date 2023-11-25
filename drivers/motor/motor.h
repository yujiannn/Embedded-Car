#ifndef MOTOR_H
#define MOTOR_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
// Define the pins for motor control
#define MOTOR_ENA_PIN 2   // Enable A 
#define MOTOR_OUT1_PIN 3   // Motor 1 input 1
#define MOTOR_OUT2_PIN 4   // Motor 1 input 2
#define MOTOR_OUT3_PIN 19  // Motor 2 input 1
#define MOTOR_OUT4_PIN 18  // Motor 2 input 2
#define MOTOR_ENB_PIN 0  // Enable B 

// Other pin definitions (e.g., PWM, ADC, UART)
//#define PWM_PIN 5
// #define ADC_PIN 26
#define SAMPLING_INTERVAL_MS 25
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17




void initMotorPins();

void stopMotors();

void moveForward();
void moveBackward();
void moveLeft(int degrees);

void moveRight(int degrees);

bool repeating_timer_callback(struct repeating_timer *t) ;

#endif 