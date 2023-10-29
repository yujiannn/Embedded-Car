/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

static char event_str[128];
static int leftWheel_hole_count=0; //holes in left wheel disc
static int rightWheel_hole_count=0; //holes in right wheel disc
static float wheel_circumference = 10.0/100;
static absolute_time_t leftWheel_last_interrupt_time;
static absolute_time_t rightWheel_last_interrupt_time;

static float total_distance_covered = 0;
static float total_speed = 0;


void gpio_event_string(char *buf, uint32_t events);

void gpio_callback_left(uint gpio, uint32_t events) {
    leftWheel_hole_count++;

    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
    leftWheel_last_interrupt_time = current_time;
    float distance_per_pulse = wheel_circumference / 20;
    float time_per_pulse_s = time_diff_us / 1e6;
    float leftSpeed = distance_per_pulse / time_per_pulse_s;

    float distance_covered = (wheel_circumference / 20) * leftWheel_hole_count;
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    printf("GPIO %d %s\n", gpio, event_str);
    printf("Left Wheel - Distance covered: %f meters", distance_covered);
    printf(" Speed: %f meters per second", leftSpeed);

    // Update total distance and speed
    total_distance_covered += distance_covered;
    total_speed += leftSpeed;
}

void gpio_callback_right(uint gpio, uint32_t events) {
    rightWheel_hole_count++;

    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);
    rightWheel_last_interrupt_time = current_time;
    float distance_per_pulse = wheel_circumference / 20;
    float time_per_pulse_s = time_diff_us / 1e6;
    float rightSpeed = distance_per_pulse / time_per_pulse_s;
    float distance_covered = (wheel_circumference / 20) * rightWheel_hole_count;
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    printf("GPIO %d %s\n", gpio, event_str);
    printf("Right Wheel - Distance covered: %f meters", distance_covered);
    printf(" Speed: %f meters per second", rightSpeed);

    // Update total distance and speed
    total_distance_covered += distance_covered;
    total_speed += rightSpeed;
}

//average distance and speed
void print_average() {
    float average_distance = total_distance_covered / (leftWheel_hole_count + rightWheel_hole_count);
    float average_speed = total_speed / (leftWheel_hole_count + rightWheel_hole_count);

    printf("Average Distance covered: %f meters", average_distance);
    printf(" Average Speed: %f meters per second", average_speed);
}

int main() {
    stdio_init_all();
   
    gpio_set_irq_enabled_with_callback(1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback_left);
    gpio_set_irq_enabled_with_callback(3, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback_right);
    leftWheel_last_interrupt_time = get_absolute_time();
    rightWheel_last_interrupt_time = get_absolute_time();

    // Wait forever
    while(1){
        print_average();
        sleep_ms(1000);
    }
    
}


static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}