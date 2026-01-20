/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

namespace {
#define PERIOD_MS 2000ms
}

DigitalOut myled(LED1);
InterruptIn button(BUTTON1);
Timer t;

void change(){
    t.reset();
    t.start();
    myled = 1-myled;
}

void flip(){
    myled = 1-myled;
    t.stop();
}

int main()
{
    myled=0;
    if (myled.is_connected()) {
        printf("myled is connected and initialized! \n\r");
    }
    button.rise(change);
    button.fall(flip);
    while (1) {
        ThisThread::sleep_for(200ms);
        
        printf("The time taken was %llu milliseconds\n", (t.elapsed_time()).count()/1000);
    }
    
}

