/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "bme280.h"

namespace {
#define PERIOD_MS 2000ms
}

I2C i2c(I2C1_SDA, I2C1_SCL);

int main()
{
    sixtron::BME280 bme(&i2c);

    bme.initialize();
    bme.set_sampling();
    while (1) {
        printf("T = %f°C\n", bme.temperature());
        printf("P = %fPa\n", bme.pressure());
        printf("Humidite = %f%\n", bme.humidity());
        ThisThread::sleep_for(1000ms);
    }
}