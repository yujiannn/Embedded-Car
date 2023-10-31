// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/gpio.h"
// #include "hardware/timer.h"

// int timeout = 26100;

// void setupUltrasonicPins(uint trigPin, uint echoPin) {
//     gpio_init(trigPin);
//     gpio_init(echoPin);
//     gpio_set_dir(trigPin, GPIO_OUT);
//     gpio_set_dir(echoPin, GPIO_IN);
// }

// uint64_t getPulse(uint trigPin, uint echoPin) {
//     gpio_put(trigPin, 1);
//     sleep_us(10);
//     gpio_put(trigPin, 0);

//     uint64_t width = 0;
//     absolute_time_t startTime = get_absolute_time();

//     while (gpio_get(echoPin) == 0) {
//         tight_loop_contents();
//     }
//     while (gpio_get(echoPin) == 1) {
//         width++;
//         sleep_us(1);
//         if (width > timeout) return 0;
//     }
//     absolute_time_t endTime = get_absolute_time();

//     return absolute_time_diff_us(startTime, endTime);
// }

// uint64_t getCm(uint trigPin, uint echoPin) {
//     uint64_t pulseLength = getPulse(trigPin, echoPin);
//     return pulseLength * 0.0343 / 2;  // Speed of sound in air is approximately 343 m/s
// }


// uint64_t getInch(uint trigPin, uint echoPin) {
//     uint64_t pulseLength = getPulse(trigPin, echoPin);
//     return pulseLength * 0.00675;  // Speed of sound in air is approximately 13503.9 inches per second
// }

// int main() {
//     stdio_init_all();
//     uint trigPin = 0; // Replace with your TRIG pin number
//     uint echoPin = 1; // Replace with your ECHO pin number

//     setupUltrasonicPins(trigPin, echoPin);
//     printf("Ultrasonic Sensor Readings\n");

//     while (1) {
        
//         uint64_t distance_inch = getInch(trigPin, echoPin);
//         uint64_t distance_cm = getCm(trigPin, echoPin);

//         // printf("Distance: %lu cm / %lu inches\n", distance_cm, distance_inch);
//         printf("Distance: %llu cm / %llu inches\n", distance_cm, distance_inch);

        
//         sleep_ms(1000);  // Delay between measurements
//     }

//     return 0;
// }

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

int timeout = 26100;

void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

volatile uint64_t pulse_start_time = 0;
volatile uint64_t pulse_end_time = 0;

void echo_pin_isr(uint gpio, uint32_t events) {
    if (gpio_get(gpio)) {
        // Rising edge
        pulse_start_time = time_us_64();
    } else {
        // Falling edge
        pulse_end_time = time_us_64();
    }
}

uint64_t getPulse(uint trigPin, uint echoPin) {
    pulse_start_time = 0;
    pulse_end_time = 0;

    gpio_put(trigPin, 1);
    sleep_us(10);
    gpio_put(trigPin, 0);

    // Wait for falling edge interrupt
    while (pulse_end_time == 0) {
        tight_loop_contents();
    }

    // Calculate pulse duration
    uint64_t pulse_duration = pulse_end_time - pulse_start_time;

    return pulse_duration;
}

uint64_t getCm(uint trigPin, uint echoPin) {
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    return pulseLength * 0.0343 / 2;  // Speed of sound in air is approximately 343 m/s
}

// uint64_t getInch(uint trigPin, uint echoPin) {
//     uint64_t pulseLength = getPulse(trigPin, echoPin);
//     return pulseLength * 0.0135 / 2;  // Speed of sound in air is approximately 13503.9 inches per second
// }

int main() {
    stdio_init_all();
    uint trigPin = 0; // Replace with your TRIG pin number
    uint echoPin = 1; // Replace with your ECHO pin number

    setupUltrasonicPins(trigPin, echoPin);

    // Configure the ECHO pin for interrupts on both rising and falling edges
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, echo_pin_isr);

    printf("Ultrasonic Sensor Readings\n");

    while (1) {
        // uint64_t distance_inch = getInch(trigPin, echoPin);
        uint64_t distance_cm = getCm(trigPin, echoPin);

        // printf("Distance: %llu cm / %llu inches\n", distance_cm, distance_inch);
        printf("Distance: %llu cm \n", distance_cm);

        sleep_ms(1000);  // Delay between measurements
    }

    return 0;
}

