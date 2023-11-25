#include "drivers/encoder/encoder.h"
#include "drivers/ir_leftandright/ir_leftandright.h"
#include "drivers/magnometer/magnometer.h"
#include "drivers/motor/motor.h"
#include "drivers/picoserver/main/main.h"
#include "drivers/ultrasonic/ultrasonic.h"

#define TRIG_PIN 6
#define ECHO_PIN 7

int main(){
    stdio_init_all();
    initMotorPins();
    initMotorPWM();
    init_encoders();
    initIRSensorsADC();
    initialize_lsm303();
    initialize_barcode_reader();
    setupUltrasonicPins(TRIG_PIN,ECHO_PIN);
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

