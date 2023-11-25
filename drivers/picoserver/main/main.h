#ifndef MAIN_H
#define MAIN_H


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <string.h>

#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"
#include "cgi.h"


// Function declarations for barcode reading.
void initialize_barcode_reader();
void read_bar_data();

// Function declarations for wifi connection.
void initialize_wifi();

// Function declarations for the HTTP server.
void initialize_http_server();

#endif // MAIN_H

