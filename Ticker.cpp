/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

namespace {
#define PERIOD_MS 2000ms
}


Ticker flipper;
DigitalOut myled(LED1);

void flip()
{
    myled = !myled;
}

int main()
{
    myled = 1;
    flipper.attach(&flip, 2.0); // the address of the function to be attached (flip) and the interval (2 seconds)

}
