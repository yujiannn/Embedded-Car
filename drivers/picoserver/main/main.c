#include "main.h"

// Variables
//
#define BARCODE_READER_PIN 26
#define MAX_WHITE_BARS 14
#define MAX_BLACK_BARS 15
#define MAX_ENCODED_VALUES 11
#define MAX_ENCODED_STARTING_VALUES 3

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

bool read = false;

uint64_t whiteBarTime[MAX_WHITE_BARS];
uint64_t blackBarTime[MAX_BLACK_BARS];
uint64_t averageBlackBarLength = 0;
uint64_t averageWhiteBarLength = 0;
uint64_t startIntervalTime = 0;
uint64_t interval = 0;

char decodedLetter;

int correctDirectionPattern[MAX_ENCODED_STARTING_VALUES] = {1, 2, 1};
int oppositeDirectionPattern[MAX_ENCODED_STARTING_VALUES] = {0, 2, 0};
int encodedDigits[MAX_ENCODED_VALUES];
int encodedStartingDigits[MAX_ENCODED_STARTING_VALUES];
int direction;


// WIFI Credentials - take care if pushing to github!
//
const char WIFI_SSID[] = "iPhone";
const char WIFI_PASSWORD[] = "12345679";

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
    //{'I', {3, 1, 3, 0, 2, 1, 3, 0, 3, 1, 3}},
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

ArrayMapEntry 
arrayMapDictionary1[] = 
{
{'A', {3, 0, 3, 1, 2, 1, 3, 0, 3, 1, 3}},
{'B', {3, 0, 3, 1, 2, 1, 3, 0, 3, 1, 3}},
{'C', {3, 1, 3, 1, 2, 1, 3, 0, 3, 0, 3}},
{'D', {3, 0, 3, 1, 2, 1, 3, 0, 3, 1, 3}},
{'E', {3, 1, 3, 1, 2, 1, 3, 0, 3, 1, 3}},
{'F', {3, 1, 3, 1, 2, 0, 3, 0, 3, 1, 3}},
{'G', {3, 0, 3, 0, 2, 0, 3, 1, 3, 1, 3}},
{'H', {3, 1, 3, 0, 2, 0, 3, 1, 3, 1, 3}},
{'I', {3, 1, 3, 0, 2, 0, 3, 1, 3, 1, 3}},
{'J', {3, 0, 3, 1, 2, 0, 3, 1, 3, 1, 3}},
{'K', {3, 0, 2, 0, 3, 1, 3, 1, 3, 1, 3}},
{'L', {3, 0, 2, 0, 3, 1, 3, 1, 3, 1, 3}},
{'M', {3, 1, 2, 1, 3, 1, 2, 0, 3, 0, 3}},
{'N', {3, 0, 2, 0, 3, 1, 3, 1, 3, 1, 3}},
{'O', {3, 1, 2, 1, 3, 0, 2, 1, 3, 0, 3}},
{'P', {3, 1, 2, 1, 3, 0, 2, 1, 3, 0, 3}},
{'Q', {3, 0, 2, 0, 3, 1, 3, 1, 3, 1, 3}},
{'R', {3, 1, 2, 1, 3, 0, 2, 0, 3, 1, 3}},
{'S', {3, 1, 2, 1, 3, 0, 2, 0, 3, 1, 3}},
{'T', {3, 0, 2, 1, 3, 0, 3, 1, 3, 1, 3}},
{'U', {3, 0, 3, 1, 3, 1, 2, 1, 3, 0, 3}},
{'V', {3, 0, 3, 1, 3, 1, 2, 0, 3, 1, 3}},
{'W', {3, 1, 3, 1, 3, 1, 2, 0, 3, 0, 3}},
{'X', {3, 0, 3, 1, 3, 0, 2, 1, 3, 1, 3}},
{'Y', {3, 1, 3, 1, 3, 0, 2, 1, 3, 0, 3}},
{'Z', {3, 1, 3, 0, 3, 0, 2, 0, 3, 1, 3}}

};

bool compareArrays(int array1[], int array2[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (array1[i] != array2[i])
        {
            return false;
        }
    }
    return true;
}

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

char 
decode_bar_info1(int encodedString[MAX_ENCODED_VALUES])
{ 
    // Compare encoded string with arrayMapDictionary
    for (int i = 0; i < sizeof(arrayMapDictionary1) / sizeof(arrayMapDictionary1[0]); i++)
    {
        int matched = 1;
        for (int j = 0; j < MAX_ENCODED_VALUES; j++)
        {
            if (arrayMapDictionary1[i].arrayMap[j] != encodedString[j])
            {
                matched = 0;
                break;
            }
        }
        if (matched)
        {   
            return arrayMapDictionary1[i].letter;
        }
    }
    // No match found
    return '\0';
}

void
display_barcode_data()
{
    if (direction==1){
        decodedLetter=decode_bar_info(encodedString);
    }else
    decodedLetter=decode_bar_info1(encodedString);

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

     printf("\nStarting digits: ");
    
    for (int i = 0; i < MAX_ENCODED_VALUES; i++)
    {   
        encodedDigits[i]=barList[i+9].category; 
    }

    for (int i = 0; i < MAX_ENCODED_STARTING_VALUES; i++)
    {   
        encodedStartingDigits[i]=barList[i].category; 
    }


    for (int i = 0; i < MAX_ENCODED_STARTING_VALUES; i++)
    {
        //printf("%d", encodedStartingDigits[i]);
    }
    printf("\n");
    

    if (compareArrays(encodedStartingDigits, correctDirectionPattern, MAX_ENCODED_STARTING_VALUES))
    {
        printf("Barcode is in the correct direction.\n");
        direction = 1;
    }
    else if (compareArrays(encodedStartingDigits, oppositeDirectionPattern, MAX_ENCODED_STARTING_VALUES))
    {
        printf("Barcode is in the opposite direction.\n");
        direction = 0;
    }
    else
    {
        printf("Unable to detect direction.\n");
    }
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
       // uint64_t barTime = blackBarTime[i];
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
        //uint64_t barTime = whiteBarTime[i];
        // int category = (barTime > averageWhiteBarLength) ? 2 : 1;
        //printf("White Bar %d: %llu milliseconds\n", i + 1, barTime);
    }
}

void
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

        // Call to update the interface
        //
        set_decoded_result(decodedLetter);

        read = true;
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