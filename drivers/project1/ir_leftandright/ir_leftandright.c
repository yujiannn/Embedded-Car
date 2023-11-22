#include <stdio.h>
#include "pico/stdlib.h"

// Define GPIO pins connected to the left and right IR sensors.
#define LEFT_IR_SENSOR_PIN 27  
#define RIGHT_IR_SENSOR_PIN 28 

int main() {
    // Initialize the I/O for Raspberry Pi Pico. 
    stdio_init_all(); 

    // Setup the GPIO pin connected to the left IR sensor.
    gpio_init(LEFT_IR_SENSOR_PIN);  // Initialize the GPIO pin.
    gpio_set_dir(LEFT_IR_SENSOR_PIN, GPIO_IN);  // Set the GPIO as an input.
    
    // Setup the GPIO pin connected to the right IR sensor.
    gpio_init(RIGHT_IR_SENSOR_PIN);  // Initialize the GPIO pin.
    gpio_set_dir(RIGHT_IR_SENSOR_PIN, GPIO_IN);  // Set the GPIO as an input.

    // Enter an infinite loop for continuous sensor reading.
    while(1) {
        // Read the state of the left IR sensor.
        bool is_left_line_detected = gpio_get(LEFT_IR_SENSOR_PIN);        
        // Print the status of the left IR sensor.
        if(is_left_line_detected) {
            printf("Left: No Line detected!\n");
        } else {
            printf("Left: Line detected.\n");
        }
        
        // Read the state of the right IR sensor.
        bool is_right_line_detected = gpio_get(RIGHT_IR_SENSOR_PIN);        
        // Print the status of the right IR sensor.
        if(is_right_line_detected) {
            printf("Right: No Line detected!\n");
        } else {
            printf("Right: Line detected.\n");
        }
        
        // Delay for a second before the next reading.
        sleep_ms(1000); // Pause the loop for 1 second. will not use sleep for integration with the car for this
    }

    return 0; // End of the program (this line is technically unreachable due to the while(1) loop).
}
