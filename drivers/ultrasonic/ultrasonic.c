#include "pico/stdlib.h"
#include "ultrasonic_sensor.h"

// Define the trigger and echo pins
#define TRIG_PIN 6
#define ECHO_PIN 7

// Define a timeout value for the ultrasonic sensor
int timeout = 26100;

// Function to set up the Ultrasonic sensor pins
void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);  // Set TRIG pin as an output
    gpio_set_dir(echoPin, GPIO_IN);   // Set ECHO pin as an input
}

// Variables to hold the start and end times of the ultrasonic pulse
volatile uint64_t pulse_start_time = 0;
volatile uint64_t pulse_end_time = 0;

// Interrupt service routine for the ECHO pin
void echo_pin_isr(uint gpio, uint32_t events) {
    if (gpio_get(gpio)) {
        // Rising edge
        pulse_start_time = time_us_64();  // Record the time of the rising edge
    } else {
        // Falling edge
        pulse_end_time = time_us_64();  // Record the time of the falling edge
    }
}

// Function to get the duration of the ultrasonic pulse
uint64_t getPulse(uint trigPin, uint echoPin) {
    pulse_start_time = 0;
    pulse_end_time = 0;

    gpio_put(trigPin, 1);   // Send a high signal to the TRIG pin
    sleep_us(10);           // Wait for a short time
    gpio_put(trigPin, 0);   // Turn off the TRIG pin

    // Wait for the falling edge interrupt
    while (pulse_end_time == 0) {
        tight_loop_contents();
    }

    // Calculate the pulse duration
    uint64_t pulse_duration = pulse_end_time - pulse_start_time;

    return pulse_duration;
}

// Function to convert pulse duration to distance in centimeters
double getCm(uint trigPin, uint echoPin) {
    uint64_t pulseLength = getPulse(trigPin, echoPin);
    double distance_cm = (double)(pulseLength * 0.0343 / 2);  // Speed of sound in air is approximately 343 m/s
    return distance_cm;
}

void ultrasonicSensorRead(uint trigPin, uint echoPin) {
    setupUltrasonicPins(trigPin, echoPin);

    // Configure the ECHO pin for interrupts on both rising and falling edges
    gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, echo_pin_isr);

    printf("Ultrasonic Sensor Readings\n");

    // Replace the 'while(1)' with a single measurement for modularity
    double distance_cm = getCm(trigPin, echoPin);
    printf("Distance: %.2f cm\n", distance_cm);  // Format the output to two decimal places
    // Removed the sleep_ms(1000) to allow external control over read frequency
}
