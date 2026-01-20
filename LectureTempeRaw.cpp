#include "mbed.h"

I2C i2c(I2C1_SDA, I2C1_SCL);
const int BME_ADDR = 0x76 << 1;

int main() {
    char val[3];
    char cmd = 0xFA;

    printf("Début\n");

    while (1) {
        i2c.write(BME_ADDR, &cmd, 1, true);
        i2c.read(BME_ADDR, val, 3);
        uint32_t raw_temp = (val[0] << 12) | (val[1] << 4) | (val[2] >> 4);
        printf("RAW_T = %lu\n", raw_temp);
        ThisThread::sleep_for(1000ms);
    }
}
