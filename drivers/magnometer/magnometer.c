#include "magnetometer.h"
#include <math.h>

#define SDAPIN 14
#define SLKPIN 15

void init_i2c_pins(void) {
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(SDAPIN, GPIO_FUNC_I2C);
    gpio_set_function(SLKPIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDAPIN);
    gpio_pull_up(SLKPIN);
}

//Write to I2C
void write_byte(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c0, address, data, 2, false);
}

//Read from I2C
uint8_t read_byte(uint8_t address, uint8_t reg) {
    uint8_t result;
    reg |= (1 << 7);
    i2c_write_blocking(i2c0, address, &reg, 1, true);
    i2c_read_blocking(i2c0, address, &result, 1, false);
    return result;
}

//Read high/low byte
int16_t read_data(uint8_t address, uint8_t reg_high, uint8_t reg_low) {
    uint8_t hi = read_byte(address, reg_high);
    uint8_t lo = read_byte(address, reg_low);
    return (int16_t)((hi << 8) | lo);
}

//Initialize lsm303
void initialize_lsm303() {
    write_byte(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, 0x27); //Bring accelerometer to normal operation
    //write_byte(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG4_A, 0x40); //keep a full scale range ±2 gauss in continuous data update mode and change the little-endian to a big-endian structure.
    write_byte(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_CRA_REG_M, 0x14); //Change odr from 15hz to 30hz
    write_byte(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_MR_REG_M, 0x00); //Continuous mode 
}


float getPitch(int16_t Ax, int16_t Ay, int16_t Az) {
    return atan2(Ay, sqrt(Ax * Ax + Az * Az));
}

float getRoll(int16_t Ax, int16_t Az) {
    return atan2(-Ax, Az);
}

float getYaw(int16_t Hx, int16_t Hy, int16_t Hz, float pitch, float roll) {
    float cosRoll = cos(roll);
    float sinRoll = sin(roll);
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);

    float Xh = Hx * cosPitch + Hz * sinPitch;
    float Yh = Hx * sinRoll * sinPitch + Hy * cosRoll - Hz * sinRoll * cosPitch;

    return atan2(Yh, Xh);
}
float calculate_heading(int16_t Hx, int16_t Hy) {
    float heading = atan2(Hy, Hx); //Calculate heading

    // Convert from radians to degrees
    heading *= 180.0 / M_PI;

    // Adjust for declination
    float declination_angle = 0.06; //0.06 is Singapore's declination
    heading += declination_angle;

    heading = fmod(heading + 360.0, 360.0); //Normalization to keep it within 360
    return heading;
}
