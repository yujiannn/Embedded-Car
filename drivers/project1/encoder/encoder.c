#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#define leftEncoderPin 16
#define rightEncoderPin 17

static int leftWheel_hole_count = 0; // Holes in left wheel disc
static int rightWheel_hole_count = 0; // Holes in right wheel disc
static float wheel_circumference = 204.1 / 100; // Wheel circumference in meters

static absolute_time_t leftWheel_last_interrupt_time;
static absolute_time_t rightWheel_last_interrupt_time;

void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t current_time = get_absolute_time();
    float distance_per_pulse = wheel_circumference / 20;
    float time_diff_us, time_per_pulse_s, distance_covered, speed;

    if (gpio == leftEncoderPin) { // Left wheel encoder
        if (gpio_get(gpio)) {
            leftWheel_hole_count++;
        }
        time_diff_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
        leftWheel_last_interrupt_time = current_time;
        time_per_pulse_s = time_diff_us / 1e6;
        speed = distance_per_pulse / time_per_pulse_s;
        distance_covered = distance_per_pulse * leftWheel_hole_count;
        printf("Left Wheel - Distance: %f meters, Speed: %f meters per second\n", distance_covered, speed);
    } else if (gpio == rightEncoderPin) { // Right wheel encoder
        if (gpio_get(gpio)) {
            rightWheel_hole_count++;
        }
        time_diff_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);
        rightWheel_last_interrupt_time = current_time;
        time_per_pulse_s = time_diff_us / 1e6;
        speed = distance_per_pulse / time_per_pulse_s;
        distance_covered = distance_per_pulse * rightWheel_hole_count;
        printf("Right Wheel - Distance: %f meters, Speed: %f meters per second\n", distance_covered, speed);
    }
}

void reset() {
    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff_left_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
    int64_t time_diff_right_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);

    if (time_diff_left_us > 2e6) { // More than 2 seconds without a pulse
        leftWheel_hole_count = 0;
        leftWheel_last_interrupt_time = current_time;
    }
    if (time_diff_right_us > 2e6) { // More than 2 seconds without a pulse
        rightWheel_hole_count = 0;
        rightWheel_last_interrupt_time = current_time;
    }
}

void init() {
    stdio_init_all();
    gpio_pull_up(leftEncoderPin);
    gpio_pull_up(rightEncoderPin);
    gpio_set_irq_enabled_with_callback(leftEncoderPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(rightEncoderPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    leftWheel_last_interrupt_time = get_absolute_time();
    rightWheel_last_interrupt_time = get_absolute_time();
}

int main() {
    init();
    while (1) {
        reset();
        sleep_ms(1000);
    }
}
