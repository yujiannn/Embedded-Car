#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

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

int main() {
    stdio_init_all();
    initMotorPins();
    

    gpio_set_function(MOTOR_ENA_PIN, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR_ENB_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(MOTOR_ENA_PIN);
    uint slice_numB = pwm_gpio_to_slice_num(MOTOR_ENB_PIN);
    pwm_set_clkdiv(slice_num, 100);
    pwm_set_clkdiv(slice_numB, 100);

    pwm_set_wrap(slice_num, 62500);
    pwm_set_wrap(slice_numB, 62500);

    pwm_set_chan_level(slice_num, PWM_CHAN_A, 62500 / 2);
    pwm_set_chan_level(slice_numB, PWM_CHAN_B, 62500 / 2);

    pwm_set_gpio_level(PWM_CHAN_A, 31250);


    pwm_set_enabled(slice_num, true);
    pwm_set_enabled(slice_numB, true);

    while (1) {
        moveForward();
        sleep_ms(2000);
        stopMotors();
        sleep_ms(1000);

        moveBackward();
        sleep_ms(2000);
        stopMotors();
        sleep_ms(1000);

        moveLeft(45);
        sleep_ms(2000);
        stopMotors();
        sleep_ms(1000);

        moveRight(90);
        sleep_ms(2000);
        stopMotors();
        sleep_ms(1000);

    }

    return 0;
}
