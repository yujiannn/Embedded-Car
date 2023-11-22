#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include <string.h>

#define MAX_WHITE_BARS 14
#define MAX_BLACK_BARS 15
#define MAX_ENCODED_VALUES 11

static int whiteBarCount = -1;
static int blackBarCount = -1;
static uint64_t whiteBarTime[MAX_WHITE_BARS];
static uint64_t blackBarTime[MAX_BLACK_BARS];
static uint64_t startIntervalTime = 0;
static uint64_t interval = 0;

typedef struct
{
    uint64_t startTime;
    int category;
} BarInfo;

typedef struct
{
    char letter;
    int arrayMap[MAX_ENCODED_VALUES];
} ArrayMapEntry;

ArrayMapEntry arrayMapDictionary[] = {
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

char decodeBarInfo(int encodedString[MAX_ENCODED_VALUES])
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

int main(void)
{

    stdio_init_all();
    gpio_init(26);
    gpio_set_dir(26, GPIO_IN);

    sleep_ms(2000);

    int previousState = gpio_get(26);
    while (1)
    {
        int currentState = gpio_get(26);

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
            break;
        }
        previousState = currentState;
    }

    // Calculate average white bar length
    uint64_t totalWhiteBarTime = 0;
    for (int i = 0; i < MAX_WHITE_BARS; i++)
    {
        totalWhiteBarTime += whiteBarTime[i];
    }
    uint64_t averageWhiteBarLength = MAX_WHITE_BARS > 0 ? totalWhiteBarTime / MAX_WHITE_BARS : 0;

    // Determine if each white bar is thin or thick based on the average
    //printf("White Bar Intervals:\n");
    for (int i = 0; i <= MAX_WHITE_BARS; i++)
    {
        uint64_t barTime = whiteBarTime[i];
        // int category = (barTime > averageWhiteBarLength) ? 2 : 1;
        //printf("White Bar %d: %llu milliseconds\n", i + 1, barTime);
    }

    // Calculate average black bar length
    uint64_t totalBlackBarTime = 0;
    for (int i = 0; i < MAX_BLACK_BARS; i++)
    {
        totalBlackBarTime += blackBarTime[i];
    }
    uint64_t averageBlackBarLength = MAX_BLACK_BARS > 0 ? totalBlackBarTime / MAX_BLACK_BARS : 0;

    // Determine if each black bar is thin or thick based on the average
    //printf("Black Bar Intervals:\n");
    for (int i = 0; i < MAX_BLACK_BARS; i++)
    {
        uint64_t barTime = blackBarTime[i];
        // int category = (barTime > averageBlackBarLength) ? 2 : 1;
        //printf("Black Bar %d: %llu milliseconds\n", i + 1, barTime);
    }

    // Store the start time and category of each bar
    BarInfo barList[MAX_WHITE_BARS + MAX_BLACK_BARS];
    int barCount = 0;
    int whiteIndex = 0;
    int blackIndex = 0;
    

    

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

    int encodedString[MAX_ENCODED_VALUES];
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

    
    
    whiteBarCount = 0;
    blackBarCount = 0;
    startIntervalTime = 0;
    interval = 0;
    
    char decodedLetter = decodeBarInfo(encodedString);

    if (decodedLetter != '\0')
    {
        printf("Decoded Letter: %c\n", decodedLetter);
    }
    else
    {
        printf("No letter matched the encoded string.\n");
    }


    return 0;
}