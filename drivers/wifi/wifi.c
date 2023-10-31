#include "pico/cyw43_arch.h"
#include <stdio.h>
#include "pico/stdlib.h"
//#include "hardware/gpio.h"
//#include "hardware/timer.h"
//#include "lwip/ip4_addr.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ping.h"
#include "message_buffer.h"

#define TRIG_PIN 0   // Ultrasonic sensor TRIG pin
#define ECHO_PIN 1   // Ultrasonic sensor ECHO pin

// #define WIFI_SSID "YourWiFiSSID"      // Replace with your Wi-Fi SSID
// #define WIFI_PASSWORD "YourPassword"  // Replace with your Wi-Fi password

#define TIMEOUT 26100
#define mbaTASK_MESSAGE_BUFFER_SIZE (60)

#ifndef PING_ADDR
#define PING_ADDR "142.251.35.196"
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

static MessageBufferHandle_t xControlMessageBuffer;

// void setupUltrasonicPins(uint trigPin, uint echoPin) {
//     gpio_init(trigPin);
//     gpio_init(echoPin);
//     gpio_set_dir(trigPin, GPIO_OUT);
//     gpio_set_dir(echoPin, GPIO_IN);
// }

// volatile uint64_t pulse_start_time = 0;
// volatile uint64_t pulse_end_time = 0;

// void echo_pin_isr(uint gpio, uint32_t events) {
//     if (gpio_get(gpio)) {
//         // Rising edge
//         pulse_start_time = time_us_64();
//     } else {
//         // Falling edge
//         pulse_end_time = time_us_64();
//     }
// }

// uint64_t getPulse(uint trigPin) {
//     pulse_start_time = 0;
//     pulse_end_time = 0;

//     gpio_put(trigPin, 1);
//     sleep_us(10);
//     gpio_put(trigPin, 0);

//     // Wait for falling edge interrupt
//     while (pulse_end_time == 0) {
//         tight_loop_contents();
//     }

//     // Calculate pulse duration
//     uint64_t pulse_duration = pulse_end_time - pulse_start_time;

//     return pulse_duration;
// }

// uint64_t getCm(uint trigPin, uint echoPin) {
//     uint64_t pulseLength = getPulse(trigPin);
//     return pulseLength * 0.0343 / 2;  // Speed of sound in air is approximately 343 m/s
// }

// void ultrasonic_task(void *params) {
//     uint trigPin = 0; // Replace with your TRIG pin number
//     uint echoPin = 1; // Replace with your ECHO pin number

//     setupUltrasonicPins(trigPin, echoPin);

//     // Configure the ECHO pin for interrupts on both rising and falling edges
//     gpio_set_irq_enabled_with_callback(echoPin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, echo_pin_isr);

//     printf("Ultrasonic Sensor Readings\n");

//     while (1) {
//         uint64_t distance_cm = getCm(trigPin, echoPin);
//         printf("Distance: %llu cm\n", distance_cm);
//         vTaskDelay(pdMS_TO_TICKS(1000));  // Delay between measurements
//     }
// }

void wifi_task(void *params) {
    if (cyw43_arch_init()) {
        printf("failed to initialize\n");
        vTaskDelete(NULL);
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        vTaskDelete(NULL);
    } else {
        printf("Connected.\n");
    }

    ip_addr_t ping_addr;
    ipaddr_aton(PING_ADDR, &ping_addr);
    ping_init(&ping_addr);

    while (1) {
        // not much to do as LED is in another task, and we're using RAW (callback) lwIP API
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    cyw43_arch_deinit();
}

int main() {
    stdio_init_all();

    xControlMessageBuffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    xTaskCreate(wifi_task, "WifiTask", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, NULL);
    //xTaskCreate(ultrasonic_task, "UltrasonicTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    return 0;
}
