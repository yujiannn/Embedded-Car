#include "ir_sensors.h"
#include <stdio.h>

// Initialize the ADC for IR sensors
void initIRSensorsADC() {
    adc_init();
    
    // Initialize the ADC pins for left and right IR sensors
    adc_gpio_init(IR_SENSOR_LEFT_GPIO_PIN);
    adc_gpio_init(IR_SENSOR_RIGHT_GPIO_PIN);

    // Set up a reasonable ADC clock divisor
    adc_set_clkdiv(8);
}

// Read from an ADC channel
uint16_t readFromADCChannel(uint channel) {
    adc_select_input(channel);
    return adc_read();
}

// Function to check the line following logic once and return sensor states
int checkLineSensors() {
    // Read the analog value from the left IR sensor
    uint16_t left_sensor_value = readFromADCChannel(LEFT_IR_SENSOR_ADC_CHANNEL);
    
    // Read the analog value from the right IR sensor
    uint16_t right_sensor_value = readFromADCChannel(RIGHT_IR_SENSOR_ADC_CHANNEL);
    
    // Adjust these threshold values accordingly
    uint16_t threshold = 500; // Example threshold

    // Determine if a black line is detected based on the sensor values
    if (left_sensor_value >= threshold && right_sensor_value >= threshold) {
        printf(" Both lines detect black\n");
        return 1; // both line is detected
    } else {
        printf("no black lines detected\n");
        return 0; // No line is detected
    }
}

// Function for debugging IR sensors
void debugIRSensors() {
    while (1) {
        uint16_t left_sensor_value = readFromADCChannel(LEFT_IR_SENSOR_ADC_CHANNEL);
        uint16_t right_sensor_value = readFromADCChannel(RIGHT_IR_SENSOR_ADC_CHANNEL);
        printf("Left sensor: %u, Right sensor: %u\n", left_sensor_value, right_sensor_value);
        sleep_ms(500);
    }
}