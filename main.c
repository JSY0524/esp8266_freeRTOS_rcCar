/*
 * Example of using ultrasonic rnaghe meter like HC-SR04
 *
 * Part of esp-open-rtos
 * Copyright (C) 2016 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
 
/*
 * Softuart example
 *
 * Copyright (C) 2017 Ruslan V. Uss <unclerus@gmail.com>
 * Copyright (C) 2016 Bernhard Guillon <Bernhard.Guillon@web.de>
 * Copyright (c) 2015 plieningerweb
 *
 * MIT Licensed as described in the file LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "esp8266.h"
#include <esp/gpio.h>
#include "ultrasonic/ultrasonic.h"
#include "softuart/softuart.h"
#include "pwm.h"

#define RX_PIN 3
#define TX_PIN 1

#define TRIGGER_PIN 14	//D5
#define ECHO_PIN    12	//D6

#define MAX_DISTANCE_CM 500 // 5m max

int32_t distance = 0;

void task1(void *pvParameters)
{
    QueueHandle_t *queue = (QueueHandle_t *)pvParameters;
    // setup software uart to 9600 8n1
    softuart_open(0, 9600, RX_PIN, TX_PIN);
    uint32_t count = 0;
    while (true)
    {
        if (!softuart_available(0))
            continue;

        char c = softuart_read(0);
	
	if(c=='x') softuart_put(0,distance);
	else softuart_put(0, c);

        vTaskDelay(100);
        xQueueSend(*queue, &count, 0);
        count++;
    }
}

void task2(void *pvParameters)
{
    QueueHandle_t *queue = (QueueHandle_t *)pvParameters;

    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_PIN,
        .echo_pin = ECHO_PIN
    };

    ultrasoinc_init(&sensor);

    uint32_t count = 0;

    while(1) {
	uint32_t distance_temp = ultrasoinc_measure_cm(&sensor, MAX_DISTANCE_CM);
        if (distance_temp < 0)
	    break;
        else distance = distance_temp;
	  
//            printf("Distance: %d cm, %.02f m\n", distance, distance / 100.0);
        vTaskDelay(100);
        xQueueSend(*queue, &count, 0);
        count++;
    }

}


static QueueHandle_t mainqueue;

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    mainqueue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(task1, "tsk1", 256, &mainqueue, 2, NULL);
    xTaskCreate(task2, "tsk2", 256, &mainqueue, 3, NULL);
}



