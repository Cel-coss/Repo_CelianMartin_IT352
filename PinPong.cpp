#include "mbed.h"

DigitalOut led1(LED1);

Thread thread1;
Thread thread2;
Mutex stdio_mutex;

void ping_thread()
{
    for(int i = 0; i < 100; i++){
        stdio_mutex.lock();
        printf("Ping!\n");
        stdio_mutex.unlock();
    }
}

void pong_thread()
{
    for(int i = 0; i < 100; i++){
        stdio_mutex.lock();
        printf("Pong!\n");
        stdio_mutex.unlock();
    }
}

int main()
{
    thread1.start(ping_thread);
    thread2.start(pong_thread);

    osThreadSetPriority(osThreadGetId(), osPriorityBelowNormal);

    while (true) {
        led1 = !led1;
        stdio_mutex.lock();
        printf("Alive\n");
        stdio_mutex.unlock();
        ThisThread::sleep_for(500ms);

    }
}
