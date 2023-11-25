#include "motor.h"


int degrees_to_turn = 0;

// Initialize the GPIO pins for motor control
void initMotorPins() {
    gpio_init(MOTOR_ENA_PIN);
    gpio_init(MOTOR_ENB_PIN);
    gpio_init(MOTOR_OUT1_PIN);
    gpio_init(MOTOR_OUT2_PIN);
    gpio_init(MOTOR_OUT3_PIN);
    gpio_init(MOTOR_OUT4_PIN);

    // Set the GPIO pins as outputs
    // gpio_set_dir(MOTOR_ENA_PIN, GPIO_OUT);
    // gpio_set_dir(MOTOR_ENB_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_OUT1_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_OUT2_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_OUT3_PIN, GPIO_OUT);
    gpio_set_dir(MOTOR_OUT4_PIN, GPIO_OUT);
}

void stopMotors() {
    gpio_put(MOTOR_ENA_PIN, 0); // Disable both motors
    gpio_put(MOTOR_ENB_PIN, 0);
}

void moveForward() {
    gpio_put(MOTOR_OUT1_PIN, 1);
    gpio_put(MOTOR_OUT2_PIN, 0);
    gpio_put(MOTOR_OUT3_PIN, 1);
    gpio_put(MOTOR_OUT4_PIN, 0);
    // // Enable both motors
    // gpio_put(MOTOR_ENA_PIN, 1);
    // gpio_put(MOTOR_ENB_PIN, 1); 
}

void moveBackward() {
    gpio_put(MOTOR_OUT1_PIN, 0);
    gpio_put(MOTOR_OUT2_PIN, 1);
    gpio_put(MOTOR_OUT3_PIN, 0);
    gpio_put(MOTOR_OUT4_PIN, 1);
    // gpio_put(MOTOR_ENA_PIN, 1); // Enable both motors
    // gpio_put(MOTOR_ENB_PIN, 1);
}

void moveLeft(int degrees) {
    gpio_put(MOTOR_OUT1_PIN, 0);
    gpio_put(MOTOR_OUT2_PIN, 0);
    gpio_put(MOTOR_OUT3_PIN, 1);
    gpio_put(MOTOR_OUT4_PIN, 0);
    degrees_to_turn = degrees;
    // gpio_put(MOTOR_ENA_PIN, 1); // Enable both motors
    // gpio_put(MOTOR_ENB_PIN, 1);
}

void moveRight(int degrees) {
    gpio_put(MOTOR_OUT1_PIN, 1);
    gpio_put(MOTOR_OUT2_PIN, 0);
    gpio_put(MOTOR_OUT3_PIN, 0);
    gpio_put(MOTOR_OUT4_PIN, 0);

    degrees_to_turn = degrees;
    // gpio_put(MOTOR_ENA_PIN, 1); // Enable both motors
    // gpio_put(MOTOR_ENB_PIN, 1);
}

bool repeating_timer_callback(struct repeating_timer *t) {
    uint32_t result = adc_read();
    uint64_t current_time_us = time_us_64();
    uint32_t milliseconds = (uint32_t)(current_time_us / 1000);
    uint32_t seconds = milliseconds / 1000 % 60;
    uint32_t minutes = (milliseconds / (1000 * 60)) % 60;
    uint32_t hours = (milliseconds / (1000 * 60 * 60)) % 24;
    milliseconds %= 1000;
    printf("%02u:%02u:%02u:%03u -> ADC value: %d\n", hours, minutes, seconds, milliseconds, result);
    return true; // Keep the timer repeating
}

