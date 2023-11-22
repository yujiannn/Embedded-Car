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

// Variables
//
#define BARCODE_READER_PIN 26
#define MAX_WHITE_BARS 14
#define MAX_BLACK_BARS 15
#define MAX_ENCODED_VALUES 11

// Barcode struct
//
typedef 
struct
{
    uint64_t startTime;
    int category;
} BarInfo;

// Store the value being read
//
typedef 
struct
{
    char letter;
    int arrayMap[MAX_ENCODED_VALUES];
} ArrayMapEntry;

int whiteBarCount = -1;
int blackBarCount = -1;
int previousState = 0;
int barCount = 0;
int whiteIndex = 0;
int blackIndex = 0;

int encodedString[MAX_ENCODED_VALUES];
BarInfo barList[MAX_WHITE_BARS + MAX_BLACK_BARS];

uint64_t whiteBarTime[MAX_WHITE_BARS];
uint64_t blackBarTime[MAX_BLACK_BARS];
uint64_t averageBlackBarLength = 0;
uint64_t averageWhiteBarLength = 0;
uint64_t startIntervalTime = 0;
uint64_t interval = 0;


// WIFI Credentials - take care if pushing to github!
//
const char WIFI_SSID[] = "Galaxy eee";
const char WIFI_PASSWORD[] = "kuiv4369";

// Match data to letter
// 
ArrayMapEntry 
arrayMapDictionary[] = 
{
    {'A', {3, 0, 3, 1, 3, 1, 2, 1, 3, 0, 3}},
    {'B', {3, 1, 3, 0, 3, 1, 2, 1, 3, 0, 3}},
    {'C', {3, 0, 3, 0, 3, 1, 2, 1, 3, 1, 3}},
    {'D', {3, 1, 3, 1, 3, 0, 2, 1, 3, 0, 3}},
    {'E', {3, 0, 3, 1, 3, 0, 2, 1, 3, 1, 3}},
    {'F', {3, 1, 3, 0, 3, 0, 2, 1, 3, 1, 3}},
    {'G', {3, 1, 3, 1, 3, 1, 2, 0, 3, 0, 3}},
    {'H', {3, 0, 3, 1, 3, 1, 2, 0, 3, 1, 3}},
    {'I', {3, 1, 3, 0, 3, 1, 2, 0, 3, 1, 3}},
    {'J', {3, 1, 3, 1, 3, 0, 2, 0, 3, 1, 3}},
    {'K', {3, 0, 3, 1, 3, 1, 3, 1, 2, 0, 3}},
    {'L', {3, 1, 3, 0, 3, 1, 3, 1, 2, 0, 3}},
    {'M', {3, 0, 3, 0, 3, 1, 3, 1, 2, 1, 3}},
    {'N', {3, 1, 3, 1, 3, 0, 3, 1, 2, 0, 3}},
    {'O', {3, 0, 3, 1, 3, 0, 3, 1, 2, 1, 3}},
    {'P', {3, 1, 3, 0, 3, 0, 3, 1, 2, 1, 3}},
    {'Q', {3, 1, 3, 1, 3, 1, 3, 0, 2, 0, 3}},
    {'R', {3, 0, 3, 1, 3, 1, 3, 0, 2, 1, 3}},
    {'S', {3, 1, 3, 0, 3, 1, 3, 0, 2, 1, 3}},
    {'T', {3, 1, 3, 1, 3, 0, 3, 0, 2, 1, 3}},
    {'U', {3, 0, 2, 1, 3, 1, 3, 1, 3, 0, 3}},
    {'V', {3, 1, 2, 0, 3, 1, 3, 1, 3, 0, 3}},
    {'W', {3, 0, 2, 0, 3, 1, 3, 1, 3, 1, 3}},
    {'X', {3, 1, 2, 1, 3, 0, 3, 1, 3, 0, 3}},
    {'Y', {3, 0, 2, 1, 3, 0, 3, 1, 3, 1, 3}},
    {'Z', {3, 1, 2, 0, 3, 0, 3, 1, 3, 1, 3}}
};

char 
decode_bar_info(int encodedString[MAX_ENCODED_VALUES])
{
    // Compare encoded string with arrayMapDictionary
    for (int i = 0; i < sizeof(arrayMapDictionary) / sizeof(arrayMapDictionary[0]); i++)
    {
        int matched = 1;
        for (int j = 0; j < MAX_ENCODED_VALUES; j++)
        {
            if (arrayMapDictionary[i].arrayMap[j] != encodedString[j])
            {
                matched = 0;
                break;
            }
        }
        if (matched)
        {
            return arrayMapDictionary[i].letter;
        }
    }
    // No match found
    return '\0';
}

void
display_barcode_data()
{
    char decodedLetter = decode_bar_info(encodedString);

    if (decodedLetter != '\0')
    {
        printf("Decoded Letter: %c\n", decodedLetter);
    }
    else
    {
        printf("No letter matched the encoded string.\n");
    }
}

void
store_barcode_data()
{
    for (int i = 0; i < MAX_WHITE_BARS + MAX_BLACK_BARS; i++)
    {
        if (i % 2 == 0)
        {
            // Add black bar information
            if (blackIndex < MAX_BLACK_BARS)
            {
                barList[barCount].startTime = blackBarTime[blackIndex];
                barList[barCount].category = (blackBarTime[blackIndex] > averageBlackBarLength) ? 0 : 1;
                blackIndex++;
                barCount++;
            }
        }
        else
        {
            // Add white bar information
            if (whiteIndex < MAX_WHITE_BARS)
            {
                barList[barCount].startTime = whiteBarTime[whiteIndex];
                barList[barCount].category = (whiteBarTime[whiteIndex] > averageWhiteBarLength) ? 2 : 3;
                whiteIndex++;
                barCount++;
            }
        }
    }
    // Print the stored bar information
    printf("Stored Bar Information:\n");
    for (int i = 0; i < barCount; i++)
    {
        printf("%d ", barList[i].category);
    }

    for (int i = 0; i < MAX_ENCODED_VALUES; i++)
    {   
        encodedString[i]=barList[i+9].category; 
    }

    printf("\nExtracted String: ");
    for (int i = 0; i < MAX_ENCODED_VALUES; i++)
    {
        printf("%d", encodedString[i]);
    }
    printf("\n");
}

void
calculate_black_bar()
{
    // Calculate average black bar length
    uint64_t totalBlackBarTime = 0;
    for (int i = 0; i < MAX_BLACK_BARS; i++)
    {
        totalBlackBarTime += blackBarTime[i];
    }
    
    averageBlackBarLength = MAX_BLACK_BARS > 0 ? totalBlackBarTime / MAX_BLACK_BARS : 0;

    // Determine if each black bar is thin or thick based on the average
    //printf("Black Bar Intervals:\n");
    for (int i = 0; i < MAX_BLACK_BARS; i++)
    {
        uint64_t barTime = blackBarTime[i];
        // int category = (barTime > averageBlackBarLength) ? 2 : 1;
        //printf("Black Bar %d: %llu milliseconds\n", i + 1, barTime);
    }
}

void
calculate_white_bar()
{
    // Calculate average white bar length
    uint64_t totalWhiteBarTime = 0;
    for (int i = 0; i < MAX_WHITE_BARS; i++)
    {
        totalWhiteBarTime += whiteBarTime[i];
    }
    
    averageWhiteBarLength = MAX_WHITE_BARS > 0 ? totalWhiteBarTime / MAX_WHITE_BARS : 0;

    // Determine if each white bar is thin or thick based on the average
    //printf("White Bar Intervals:\n");
    for (int i = 0; i <= MAX_WHITE_BARS; i++)
    {
        uint64_t barTime = whiteBarTime[i];
        // int category = (barTime > averageWhiteBarLength) ? 2 : 1;
        //printf("White Bar %d: %llu milliseconds\n", i + 1, barTime);
    }
}

int
read_bar_data()
{
    int currentState = gpio_get(BARCODE_READER_PIN);

    if (currentState != previousState)
    {
        if (currentState == 0)
        {
            whiteBarCount++;
            startIntervalTime = to_ms_since_boot(get_absolute_time());
            //printf("White Bar %d Detected! Count: %d\n", whiteBarCount + 1, whiteBarCount + 1);
        }
        else
        {
            blackBarCount++;
            startIntervalTime = to_ms_since_boot(get_absolute_time());
            //printf("Black Bar %d Detected! Count: %d\n", blackBarCount + 1, blackBarCount + 1);
        }
    }
    else if (currentState == 0 && whiteBarCount >= 0)
    {
        // Update white bar interval
        interval = to_ms_since_boot(get_absolute_time()) - startIntervalTime;
        whiteBarTime[whiteBarCount] = interval;
    }
    else if (currentState == 1 && blackBarCount >= 0)
    {
        // Update black bar interval
        interval = to_ms_since_boot(get_absolute_time()) - startIntervalTime;
        blackBarTime[blackBarCount] = interval;
        if (blackBarCount == MAX_BLACK_BARS - 1)
        {
            if (currentState == 0)
            {
                interval = to_ms_since_boot(get_absolute_time()) - startIntervalTime;
                blackBarTime[blackBarCount] = interval;
            }
        }
    }
    if (whiteBarCount == MAX_WHITE_BARS)
    {
        calculate_white_bar();
        calculate_black_bar();
        store_barcode_data();
        display_barcode_data();
        return 0;
    }
    previousState = currentState;
}

void
initialize_barcode_reader()
{
    gpio_init(BARCODE_READER_PIN);
    gpio_set_dir(BARCODE_READER_PIN, GPIO_IN);

    previousState = gpio_get(BARCODE_READER_PIN);
}

void 
intitialize_wifi() 
{
    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    //
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0){
        printf("Attempting to connect...\n");
    }
    
    printf("Connected! \n"); // Print a success message once connected
    
    httpd_init(); // Initialise web server
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
    //
    ssi_init(); 
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    read_bar_data();
}

int 
main() 
{
    stdio_init_all();

    intitialize_wifi();

    // change delay accordingly
    initialize_barcode_reader();
    
    sleep_ms(2000);
    printf("ready to read barcode\n");
    

    // Infinite loop
    while(1)
    {
        
        read_bar_data();
    };
}