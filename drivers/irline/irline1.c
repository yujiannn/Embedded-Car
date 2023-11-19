#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/time.h"

#define IR_SENSOR_PIN 26

/* this code will not work for small black lines but should not be a problem since the ones we using it on are thick black lines. 
    and the ir sensor can be used above the surface, unlike the barcode code which requires the ir sensor to touch the paper physically*/

int main() {
    stdio_init_all();

    // Initialize ADC
    adc_init();
    adc_gpio_init(IR_SENSOR_PIN);
    adc_select_input(0);

    // Set up a reasonable ADC clock divisor
    adc_set_clkdiv(8);  // You may need to adjust this based on your specific requirements

    // Initialize flags
    int line_detected = 0;
    int no_line_detected = 0;

    while (1) {
        // Read the analog value from the IR sensor
        uint16_t sensor_value = adc_read();

        // Print the ADC reading
        //printf("ADC Reading: %u\n", sensor_value);


        // Adjust these threshold values accordingly, can uncomment the adc reading part to check if want to adjust
        uint16_t threshold_high = 400;
        uint16_t threshold_low = 350;

        // Determine if black/ white surface is detected based on the sensor value
        if (sensor_value > threshold_high && !line_detected) {
            printf("Black surface detected! Stay within the black surface.\n");
            line_detected = 1;
            no_line_detected = 0;
        } else if (sensor_value < threshold_low && !no_line_detected) {
            printf("White surface detected. Adjust the position.\n");
            no_line_detected = 1;
            line_detected = 0;
        }

        sleep_ms(100);
    }

    return 0;
}
