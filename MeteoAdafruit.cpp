/*
 * Copyright (c) 2020, CATIE
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
#include "mbed.h"
#include <nsapi_dns.h>
#include <MQTTClientMbedOs.h>
#include "bme280.h"

namespace {
    #define AIO_USERNAME "schacece"
    #define FEED_TEMP   AIO_USERNAME "/feeds/temp"
    #define FEED_HUM    AIO_USERNAME "/feeds/hum"
    #define FEED_PRESS  AIO_USERNAME "/feeds/press"
    #define FEED_ALERT_TEMP AIO_USERNAME "/feeds/alert_temp"
}

// Peripherals
static DigitalOut led(LED1);
static DigitalIn button(BUTTON1);

I2C i2c(I2C1_SDA, I2C1_SCL);
sixtron::BME280 bme(&i2c);

// Network
NetworkInterface *network;
MQTTClient *client;

float temperature = 0.0;
float humidity    = 0.0;
float pressure    = 0.0;

Mutex data_mutex;
Mutex stdio_mutex;

// MQTT
const char* hostname = "io.adafruit.com";
int port = 1883;

// Error code
nsapi_size_or_error_t rc = 0;

// Event queue
static int id_yield;
static EventQueue main_queue(32 * EVENTS_EVENT_SIZE);

//Threads
Thread sensorThread;
Thread mqttThread;
Thread pressThread;
Thread alertThread;


/*!
 *  \brief Publish data over the corresponding adafruit MQTT topic
 *
 */
static int8_t publish(const char* topic, float value) {
    char mqttPayload[16];
    snprintf(mqttPayload, sizeof(mqttPayload), "%f", value);

    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)mqttPayload;
    message.payloadlen = strlen(mqttPayload);

    rc = client->publish(topic, message);
    if (rc != 0) {
        printf("Failed to publish: %d\n", rc);
        return rc;
    }
    return 0;
}

//threads

void sensor_thread()
{
    while (true) {
        data_mutex.lock();
        temperature = bme.temperature();
        humidity    = bme.humidity();
        pressure    = bme.pressure();
        data_mutex.unlock();

        ThisThread::sleep_for(1s);
    }
}

/*!
 *  \brief Appui bouton géré dans un thread
 */
void press_thread()
{
    int tmp = 0;
    while(true){
        if (button == 1 && tmp != button){
            tmp = button;
            stdio_mutex.lock();
            printf("P = %fPa\n", pressure);
            printf("T = %fdeg C\n", temperature);
            printf("Humidite = %f%\n", humidity);
            stdio_mutex.unlock();
            }
        else if(button == 0){tmp=0;}
    }
}


/*!
 *  \brief Yield to the MQTT client
 *
 *  On error, stop publishing and yielding
 */
static void yield(){
    rc = client->yield(100);

    if (rc != 0){
        printf("Yield error: %d\n", rc);
        main_queue.cancel(id_yield);
        main_queue.break_dispatch();
        system_reset();
    }
}


void alert_thread(){
    while(true){
        float temp_copy;

        data_mutex.lock();
        temp_copy = temperature;
        data_mutex.unlock();

        if(temp_copy > 26.0){
             stdio_mutex.lock();
             printf("Alerte: Temperature excessive!\n"); 
             stdio_mutex.unlock();
             led = !led;
         }
         else{
             led = 0;
         }

         ThisThread::sleep_for(5s); 
     }
 }


void mqtt_thread()
{
    while (true) {
        client->yield(100);

        data_mutex.lock();
        float t = temperature;
        float h = humidity;
        float p = pressure;
        data_mutex.unlock();

        publish(FEED_TEMP, t);
        publish(FEED_HUM, h);
        publish(FEED_PRESS, p);
        if (temperature > 26.0) {
            publish(FEED_ALERT_TEMP, 1.0);
        } else {
            publish(FEED_ALERT_TEMP, 0.0);
        }

        ThisThread::sleep_for(8s);
    }
}

// main() runs in its own thread in the OS
// (note the calls to ThisThread::sleep_for below for delays)

int main()
{
    led = 0;
    bme.initialize();
    bme.set_sampling();

    printf("Connecting to border router...\n");

    /* Get Network configuration */
    network = NetworkInterface::get_default_instance();
    if (!network) {
        printf("Error! No network interface found.\n");
        return 0;
    }

    /* Add DNS */
    nsapi_addr_t new_dns = {
        NSAPI_IPv6,
        { 0xfd, 0x9f, 0x59, 0x0a, 0xb1, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01 }
    };
    nsapi_dns_add_server(new_dns, "LOWPAN");

    /* Border Router connection */
    rc = network->connect();
    if (rc != 0) {
        printf("Error! net->connect() returned: %d\n", rc);
        return rc;
    }

    /* Print IP address */
    SocketAddress a;
    network->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    /* Open TCP Socket */
    TCPSocket socket;
    SocketAddress address;
    network->gethostbyname(hostname, &address);
    address.set_port(port);

    /* MQTT Connection */
    client = new MQTTClient(&socket);
    socket.open(network);
    rc = socket.connect(address);
    if(rc != 0){
        printf("Connection to MQTT broker Failed\n");
        return rc;
    }

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4;
    data.keepAliveInterval = 25;
    data.username.cstring = "schacece";
    data.password.cstring = "aio_xTiW53ojKPCLqsSitHWQY7fIaPKV";

    if (client->connect(data) != 0){
        printf("Connection to MQTT Broker Failed\n");
    }

    printf("Connected to MQTT broker\n");

    // Lancer threads
    sensorThread.start(sensor_thread);
    mqttThread.start(mqtt_thread);
    pressThread.start(press_thread);
   alertThread.start(alert_thread);


    while(1){
        ThisThread::sleep_for(10ms);
    }
}
