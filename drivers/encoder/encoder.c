/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#define leftEncoderPin 1
#define rightEncoderPin 3

static int leftWheel_hole_count=0; //holes in left wheel disc
static int rightWheel_hole_count=0; //holes in right wheel disc
static float wheel_circumference = 10.0/100;
static absolute_time_t leftWheel_last_interrupt_time;
static absolute_time_t rightWheel_last_interrupt_time;
static float leftSpeed = 0.0; // Speed of left wheel
static float rightSpeed = 0.0; // Speed of right wheel

void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t current_time = get_absolute_time();
    float distance_per_pulse = wheel_circumference / 20;
    float time_diff_us, time_per_pulse_s, distance_covered;

    if (gpio == leftEncoderPin) { // Left wheel encoder
        if (gpio_get(gpio)){
            leftWheel_hole_count++;
        }
        time_diff_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
        leftWheel_last_interrupt_time = current_time;
        distance_covered = (wheel_circumference / 20) * leftWheel_hole_count;
        printf("Left Wheel - Distance covered: %f meters ", distance_covered);
    } else if (gpio == rightEncoderPin) { // Right wheel encoder
        if (gpio_get(gpio)){
            rightWheel_hole_count++;
        }
        time_diff_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);
        rightWheel_last_interrupt_time = current_time;
        distance_covered = (wheel_circumference / 20) * rightWheel_hole_count;
        printf("Right Wheel - Distance covered: %f meters ", distance_covered);
    }

    time_per_pulse_s = time_diff_us / 1e6;
    if (gpio == 1) {
        leftSpeed = distance_per_pulse / time_per_pulse_s;
        printf(" Left Speed: %f meters per second\n", leftSpeed);
    } else if (gpio == 3) {
        rightSpeed = distance_per_pulse / time_per_pulse_s;
        printf(" Right Speed: %f meters per second\n", rightSpeed);
    }
}

void calculate_average() {
    float average_distance = ((wheel_circumference / 20) * (leftWheel_hole_count + rightWheel_hole_count)) / 2;
    float average_speed = (leftSpeed + rightSpeed) / 2;
    printf("Average Distance: %f meters ", average_distance);
    printf("Average Speed: %f meters per second\n", average_speed);
} 

void reset() {
    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff_left_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
    int64_t time_diff_right_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);
    //float average_distance = ((wheel_circumference / 20) * (leftWheel_hole_count + rightWheel_hole_count)) / 2;

    if (time_diff_left_us > 2e6 && time_diff_right_us > 2e6) { // 2 seconds = 2e6 microseconds
        //printf("Average Distance: %f meters\n", average_distance);
        //printf("Average Speed before stop: %f meters per second\n", (leftSpeed + rightSpeed) / 2);
        leftWheel_hole_count = 0;
        rightWheel_hole_count = 0;
        leftSpeed = 0.0;
        rightSpeed = 0.0;
    }
}

 void init(){
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
    // Wait forever
    while(1){
        calculate_average();
        reset();
        sleep_ms(1000);
    }
}
