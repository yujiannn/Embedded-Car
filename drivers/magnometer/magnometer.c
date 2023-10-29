#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MAGNETOMETER_ADDR 0x1E
#define MR_REG_M 0x02

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    uint8_t buffer[6];
    uint8_t register_value[] = {MR_REG_M, 0x00}; // Continuous conversion mode

    // Write to MR_REG_M to set continuous conversion mode
    i2c_write_blocking(i2c0, MAGNETOMETER_ADDR, register_value, 2, false);

    while (1) {
        // Read from OUT_X_H_M register (0x03)
        buffer[0] = 0x03;
        i2c_write_blocking(i2c0, MAGNETOMETER_ADDR, buffer, 1, true);
        i2c_read_blocking(i2c0, MAGNETOMETER_ADDR, buffer, 6, false);

        int16_t x = (buffer[0] << 8) | buffer[1];
        int16_t y = (buffer[4] << 8) | buffer[5];
        int16_t z = (buffer[2] << 8) | buffer[3];

        printf("X: %d, Y: %d, Z: %d\n", x, y, z);

        sleep_ms(1000);
    }

    return 0;
}
