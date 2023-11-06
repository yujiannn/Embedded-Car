#include "pico/cyw43_arch.h"  // Include the Pico Wi-Fi module library
#include <stdio.h>            // Include standard input/output library
#include "pico/stdlib.h"      // Include Pico standard library
#include "hardware/gpio.h"    // Include hardware GPIO library
#include "hardware/timer.h"   // Include hardware timer library
#include "lwip/ip4_addr.h"    // Include LWIP IPv4 address library
#include "FreeRTOS.h"         // Include FreeRTOS library
#include "task.h"             // Include task management library
#include "ping.h"             // Include ping library for network testing
#include "message_buffer.h"   // Include message buffer library

#ifndef PING_ADDR
#define PING_ADDR "142.251.35.196"  // Define a default IP address to ping
#endif

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)  // Define the priority of the test task

static MessageBufferHandle_t xControlMessageBuffer;  // Declare a message buffer handle

// Define a task to handle Wi-Fi operations
void wifi_task(void *params) {
    // Initialize the Pico Wi-Fi module
    if (cyw43_arch_init()) {
        printf("failed to initialize\n");
        vTaskDelete(NULL);  // Delete the task if initialization fails
    }

    // Enable the Wi-Fi station mode
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    // Attempt to connect to the Wi-Fi network with a timeout of 30 seconds
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        vTaskDelete(NULL);  // Delete the task if the connection fails
    } else {
        printf("Connected.\n");
    }

    ip_addr_t ping_addr;
    ipaddr_aton(PING_ADDR, &ping_addr);  // Convert the PING_ADDR to an IP address
    ping_init(&ping_addr);  // Initialize the ping functionality

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));  // Delay the task for 100 milliseconds
    }

    cyw43_arch_deinit();  // Deinitialize the Pico Wi-Fi module when the task exits
}

int main() {
    stdio_init_all();  // Initialize standard I/O

    xControlMessageBuffer = xMessageBufferCreate(60);  // Create a message buffer

    // Create and start the Wi-Fi task
    xTaskCreate(wifi_task, "WifiTask", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, NULL);
    vTaskStartScheduler();  // Start the FreeRTOS task scheduler

    return 0;  // The main function typically does not return in embedded systems
}
