#include "mbed.h"
#include "bme280.h"

namespace {
#define PERIOD_MS 1000ms
}

I2C i2c(I2C1_SDA, I2C1_SCL);
DigitalOut led1(LED1);
InterruptIn button(BUTTON1);

Thread thread1;
Thread thread2;
Mutex stdio_mutex;
sixtron::BME280 bme(&i2c);


void temp_hum_thread()
{
    while(true){
        stdio_mutex.lock();
        printf("T = %f  deg C\n", bme.temperature());
        printf("Humidite = %f%\n", bme.humidity());
        stdio_mutex.unlock();
        ThisThread::sleep_for(2*PERIOD_MS);
    }
}

void press_thread()
{
    int tmp = 0;
    while(true){
        if (button == 1 && tmp != button){
            tmp = button;
            stdio_mutex.lock();
            printf("P = %fPa\n", bme.pressure());
            stdio_mutex.unlock();
            }
        else if(button == 0){tmp=0;}
    }
}

int main()
{   led1=0;

    bme.initialize();
    bme.set_sampling();

    thread1.start(temp_hum_thread);
    thread2.start(press_thread);

    while (true) {
        led1 = !led1;
        ThisThread::sleep_for(5*PERIOD_MS);

    }
}
