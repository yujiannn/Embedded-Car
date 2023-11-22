#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//Write Start bit
#define LSM303_ADDRESS_ACCEL 0x19
#define LSM303_ADDRESS_MAG 0x1E

//Accel Registers
#define LSM303_REGISTER_ACCEL_CTRL_REG1_A 0x20   
#define LSM303_REGISTER_ACCEL_CTRL_REG2_A 0x21   
#define LSM303_REGISTER_ACCEL_CTRL_REG3_A 0x22   
#define LSM303_REGISTER_ACCEL_CTRL_REG4_A 0x23   
#define LSM303_REGISTER_ACCEL_CTRL_REG5_A 0x24   
#define LSM303_REGISTER_ACCEL_CTRL_REG6_A 0x25   
#define LSM303_REGISTER_ACCEL_REFERENCE_A 0x26   
#define LSM303_REGISTER_ACCEL_STATUS_REG_A 0x27   
#define LSM303_REGISTER_ACCEL_OUT_X_L_A 0x28
#define LSM303_REGISTER_ACCEL_OUT_X_H_A 0x29
#define LSM303_REGISTER_ACCEL_OUT_Y_L_A 0x2A
#define LSM303_REGISTER_ACCEL_OUT_Y_H_A 0x2B
#define LSM303_REGISTER_ACCEL_OUT_Z_L_A 0x2C
#define LSM303_REGISTER_ACCEL_OUT_Z_H_A 0x2D
#define LSM303_REGISTER_ACCEL_FIFO_CTRL_REG_A 0x2E
#define LSM303_REGISTER_ACCEL_FIFO_SRC_REG_A 0x2F
#define LSM303_REGISTER_ACCEL_INT1_CFG_A 0x30
#define LSM303_REGISTER_ACCEL_INT1_SOURCE_A 0x31
#define LSM303_REGISTER_ACCEL_INT1_THS_A 0x32
#define LSM303_REGISTER_ACCEL_INT1_DURATION_A 0x33
#define LSM303_REGISTER_ACCEL_INT2_CFG_A 0x34
#define LSM303_REGISTER_ACCEL_INT2_SOURCE_A 0x35
#define LSM303_REGISTER_ACCEL_INT2_THS_A 0x36
#define LSM303_REGISTER_ACCEL_INT2_DURATION_A 0x37
#define LSM303_REGISTER_ACCEL_CLICK_CFG_A 0x38
#define LSM303_REGISTER_ACCEL_CLICK_SRC_A 0x39
#define LSM303_REGISTER_ACCEL_CLICK_THS_A 0x3A
#define LSM303_REGISTER_ACCEL_TIME_LIMIT_A 0x3B
#define LSM303_REGISTER_ACCEL_TIME_LATENCY_A 0x3C
#define LSM303_REGISTER_ACCEL_TIME_WINDOW_A 0x3D

//Mag registers
#define LSM303_REGISTER_MAG_CRA_REG_M 0x00
#define LSM303_REGISTER_MAG_CRB_REG_M 0x01
#define LSM303_REGISTER_MAG_MR_REG_M 0x02
#define LSM303_REGISTER_MAG_OUT_X_H_M 0x03
#define LSM303_REGISTER_MAG_OUT_X_L_M 0x04
#define LSM303_REGISTER_MAG_OUT_Z_H_M 0x05
#define LSM303_REGISTER_MAG_OUT_Z_L_M 0x06
#define LSM303_REGISTER_MAG_OUT_Y_H_M 0x07
#define LSM303_REGISTER_MAG_OUT_Y_L_M 0x08
#define LSM303_REGISTER_MAG_SR_REG_Mg 0x09
#define LSM303_REGISTER_MAG_IRA_REG_M 0x0A
#define LSM303_REGISTER_MAG_IRB_REG_M 0x0B
#define LSM303_REGISTER_MAG_IRC_REG_M 0x0C
#define LSM303_REGISTER_MAG_TEMP_OUT_H_M 0x31
#define LSM303_REGISTER_MAG_TEMP_OUT_L_M 0x32
 


#ifdef i2c0

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

#endif

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
    initialize_lsm303();

    while (1) {
        int16_t Hx = read_data(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_OUT_X_H_M, LSM303_REGISTER_MAG_OUT_X_L_M);
        int16_t Hy = read_data(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_OUT_Y_H_M, LSM303_REGISTER_MAG_OUT_Y_L_M);
        int16_t Hz = read_data(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_OUT_Z_H_M, LSM303_REGISTER_MAG_OUT_Z_L_M);
        int16_t Ax = read_data(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_OUT_X_H_A, LSM303_REGISTER_ACCEL_OUT_X_L_A);
        int16_t Ay = read_data(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_OUT_Y_H_A, LSM303_REGISTER_ACCEL_OUT_Y_L_A);
        int16_t Az = read_data(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_OUT_Z_H_A, LSM303_REGISTER_ACCEL_OUT_Z_L_A);
        printf("\nLSM303DLHC Raw Data \nMagnetometer output: \nHx: %d", Hx);
        printf("\nHy: %d", Hy);
        printf("\nHz: %d", Hz);
        printf("\nAccelerometer output: \nAx: %d", Ax);
        printf("\nAy: %d", Ay);
        printf("\nAz: %d", Az);
        float pitch = getPitch(Ax, Ay, Az);
        float roll = getRoll(Ax, Az);
        float yaw = getYaw(Hx, Hy, Hz, pitch, roll);
        float heading = calculate_heading(Hx, Hy);
        printf("\nLSM303DLHC Compass Heading: %.2f degrees", heading); //Look at this for direction
        printf("\nPitch: %.2f, Roll: %.2f, Yaw (Heading): %.2f\n", pitch * 180.0 / M_PI, roll * 180.0 / M_PI, yaw * 180.0 / M_PI); 
        sleep_ms(2000); // Sleep for 2 seconds
    }

    return 0;
}