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
    #define MOTOR_ENB_PIN 0    // Enable B 
    bool stopAllMovement = false; // Flag to stop all movement when an object is too close
    double threshold_distance_cm = 10.0; // Set this to your desired threshold distance
    // Define the pins for the IR sensors
    #define IR_SENSOR_LEFT_GPIO_PIN 27
    #define IR_SENSOR_RIGHT_GPIO_PIN 28
    // Encoder pins
    #define LEFT_ENCODER_PIN 16
    #define RIGHT_ENCODER_PIN 17

    // Define the ADC channels corresponding to the IR sensor GPIO pins
    #define LEFT_IR_SENSOR_ADC_CHANNEL 1  // ADC Channel for GPIO 27
    #define RIGHT_IR_SENSOR_ADC_CHANNEL 2 // ADC Channel for GPIO 28
    int degrees_to_turn = 0;
    int timeout = 26100;
    static int leftWheel_hole_count=0; //holes in left wheel disc
    static int rightWheel_hole_count=0; //holes in right wheel disc
    static float wheel_circumference = 10.0/100;
    static absolute_time_t leftWheel_last_interrupt_time;
    static absolute_time_t rightWheel_last_interrupt_time;
    static float leftSpeed = 0.0; // Speed of left wheel
    static float rightSpeed = 0.0; // Speed of right wheel
    volatile float leftWheelSpeed = 0.0f;
    volatile float rightWheelSpeed = 0.0f;
    volatile uint64_t pulse_start_time = 0;
    volatile uint64_t pulse_end_time = 0;


    void setupUltrasonicPins(uint trigPin, uint echoPin);
    void echo_pin_isr(uint gpio, uint32_t events);
    uint64_t getPulse(uint trigPin, uint echoPin);
    double getCm(uint trigPin, uint echoPin);
    void gpio_callback(uint gpio, uint32_t events) {
        absolute_time_t current_time = get_absolute_time();
        float distance_per_pulse = wheel_circumference / 20;
        float time_diff_us, time_per_pulse_s;


        if (gpio == LEFT_ENCODER_PIN) { // Left wheel encoder
            if (gpio_get(gpio)) {
                leftWheel_hole_count++;
                time_diff_us = absolute_time_diff_us(leftWheel_last_interrupt_time, current_time);
                leftWheel_last_interrupt_time = current_time;
                time_per_pulse_s = time_diff_us / 1e6;
                leftSpeed = distance_per_pulse / time_per_pulse_s; // Calculate speed directly
                leftWheelSpeed = leftSpeed; // after calculating leftSpeed
            }
        } else if (gpio == RIGHT_ENCODER_PIN) { // Right wheel encoder
            if (gpio_get(gpio)) {
                rightWheel_hole_count++;
                time_diff_us = absolute_time_diff_us(rightWheel_last_interrupt_time, current_time);
                rightWheel_last_interrupt_time = current_time;
                time_per_pulse_s = time_diff_us / 1e6;
                rightSpeed = distance_per_pulse / time_per_pulse_s; // Calculate speed directly
                rightWheelSpeed = rightSpeed; // after calculating rightSpeed
            }
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

    // Function to initialize encoders
    void init_encoders() {
        // Initialize the GPIO pins for the encoders
        gpio_init(LEFT_ENCODER_PIN);
        gpio_init(RIGHT_ENCODER_PIN);

        // Pull up the encoder pins
        gpio_pull_up(LEFT_ENCODER_PIN);
        gpio_pull_up(RIGHT_ENCODER_PIN);

        // Set up the encoder interrupt callbacks
        gpio_set_irq_enabled_with_callback(LEFT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_set_irq_enabled_with_callback(RIGHT_ENCODER_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    }

    void initMotorPWM(){
        // Initialize PWM for Enable A
        gpio_set_function(MOTOR_ENA_PIN, GPIO_FUNC_PWM);
        uint slice_num_a = pwm_gpio_to_slice_num(MOTOR_ENA_PIN);

        // Initialize PWM for Enable B
        gpio_set_function(MOTOR_ENB_PIN, GPIO_FUNC_PWM);
        uint slice_num_b = pwm_gpio_to_slice_num(MOTOR_ENB_PIN);

        // Set PWM frequency and duty cycle (example values)
        pwm_set_clkdiv(slice_num_a, 8.0); // Adjust to your requirements
        pwm_set_clkdiv(slice_num_b, 8.0); // Adjust to your requirements

        pwm_set_wrap(slice_num_a, 65535);
        pwm_set_wrap(slice_num_b, 65535);

        pwm_set_chan_level(slice_num_a, PWM_CHAN_A, 32767); // 50% duty cycle
        pwm_set_chan_level(slice_num_b, PWM_CHAN_B, 32767); // 50% duty cycle

        pwm_set_enabled(slice_num_a, true);
        pwm_set_enabled(slice_num_b, true);
    }

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

    // Stop the motors
    void stopMotors() {
        gpio_put(MOTOR_ENA_PIN, 0); // Disable both motors
        gpio_put(MOTOR_ENB_PIN, 0);
        //printf("stopping");
    }

    // Move the car forward
    void moveForward() {
        gpio_put(MOTOR_OUT1_PIN, 1);
        gpio_put(MOTOR_OUT2_PIN, 0);
        gpio_put(MOTOR_OUT3_PIN, 1);
        gpio_put(MOTOR_OUT4_PIN, 0);
        //printf("moving straight");
    }

    // Move the car backward
    void moveBackward() {
        gpio_put(MOTOR_OUT1_PIN, 0);
        gpio_put(MOTOR_OUT2_PIN, 1);
        gpio_put(MOTOR_OUT3_PIN, 0);
        gpio_put(MOTOR_OUT4_PIN, 1);
        //printf("moving back");
    }

    // Move the car to the right
    void moveRight(int duration_ms) {
        // Depending on your motor setup, adjust this logic to turn the car right
        gpio_put(MOTOR_OUT1_PIN, 1);
        gpio_put(MOTOR_OUT2_PIN, 0);
        gpio_put(MOTOR_OUT3_PIN, 0);
        gpio_put(MOTOR_OUT4_PIN, 0);

        sleep_ms(duration_ms);
        //printf("moving right");
    }

    void moveLeft(int degree) {
        // Depending on your motor setup, adjust this logic to turn the car right
        gpio_put(MOTOR_OUT1_PIN, 0);
        gpio_put(MOTOR_OUT2_PIN, 0);
        gpio_put(MOTOR_OUT3_PIN, 1);
        gpio_put(MOTOR_OUT4_PIN, 0);
        degrees_to_turn = degree;
        //printf("moving right");
    }

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
        } //else if (left_sensor_value < threshold) {
            //printf("Right sensor detects a line\n");
            //return 1; // Only right sensor detects a line
        //} else if (right_sensor_value < threshold) {
            //printf("Left sensor detects a line\n");
            //return 2; // Only left sensor detects a line} 
            else{
            printf("no black lines detected\n");
            return 0; // Both line is detected
        }
    }

    void debugIRSensors() {
    while (1) {
        uint16_t left_sensor_value = readFromADCChannel(LEFT_IR_SENSOR_ADC_CHANNEL);
        uint16_t right_sensor_value = readFromADCChannel(RIGHT_IR_SENSOR_ADC_CHANNEL);
        printf("Left sensor: %u, Right sensor: %u\n", left_sensor_value, right_sensor_value);
        sleep_ms(500);
    }
}
    int main() {
        stdio_init_all();
        // Initialize motor pins, encoders, IR sensors, etc.
        initMotorPins();
        initMotorPWM();
        init_encoders();
        initIRSensorsADC();
        // Initialize ultrasonic sensor pins
        uint trigPin = 6;
        uint echoPin = 7;
        setupUltrasonicPins(trigPin, echoPin);
        gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, echo_pin_isr);

        gpio_set_function(MOTOR_ENA_PIN, GPIO_FUNC_PWM);
        gpio_set_function(MOTOR_ENB_PIN, GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(MOTOR_ENA_PIN);
        uint slice_numB = pwm_gpio_to_slice_num(MOTOR_ENB_PIN);
        pwm_set_clkdiv(slice_num, 100);
        pwm_set_clkdiv(slice_numB, 100);

        pwm_set_wrap(slice_num, 62500);
        pwm_set_wrap(slice_numB, 62500);

        pwm_set_chan_level(slice_num, PWM_CHAN_A, 62500 / 3);
        pwm_set_chan_level(slice_numB, PWM_CHAN_B, 62500 / 2);

        pwm_set_gpio_level(PWM_CHAN_A, 31250);


        pwm_set_enabled(slice_num, true);
        pwm_set_enabled(slice_numB, true);

        //debugIRSensors();
     while (1) {
        if (stopAllMovement) {
            stopMotors();
            break; // Exit the while loop and end the program
        }

        // Check ultrasonic sensor first
        double distance_cm = getCm(trigPin, echoPin);
        if (distance_cm <= threshold_distance_cm) {
            printf("Object detected within threshold distance. Stopping motors, moving backward.\n");
            stopMotors(); // Stop the car
            moveBackward(); // Move the car backward
            sleep_ms(2000); // Move back for 2 seconds
            stopMotors(); // Stop the car after moving back
            stopAllMovement = true; // Set flag to true to prevent further movement
        } else {
            // Check line sensors if no object is detected by ultrasonic sensor
            int sensor_status = checkLineSensors();
            switch (sensor_status) {
                case 0: // No line detected
                    moveForward(); // Move forward
                    printf("no blACK LINES, going straight");
                    break;
                case 1: // Line detected
                printf("black lines, stop and turn");
                    stopMotors(); // Stop the car
                    sleep_ms(2000); // Stop for 2 seconds
                    moveRight(200); // Turn right for 0.2 seconds
                    stopMotors(); // Stop the car
                    sleep_ms(2000); // Stop for 2 seconds
                    // No need to check the sensor status here, it will be checked at the start of the loop
                    break;
            }
        }
        sleep_ms(100); // Sleep for a short interval before checking the sensors again
    }

    return 0;
}


    // Function to set up the Ultrasonic sensor pins
    void setupUltrasonicPins(uint trigPin, uint echoPin) {
        gpio_init(trigPin);
        gpio_init(echoPin);
        gpio_set_dir(trigPin, GPIO_OUT);  // Set TRIG pin as an output
        gpio_set_dir(echoPin, GPIO_IN);   // Set ECHO pin as an input
    }

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
