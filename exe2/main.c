/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/rtc.h"
 #include "pico/util/datetime.h"
 
 
 #define TRIG_PIN_2 13
 #define ECHO_PIN_2 12

 #define TRIG_PIN 19
 #define ECHO_PIN 18

 volatile bool alarm_flag = false;
 volatile bool alarm2_flag = false;
 
 volatile bool echo_flag = false; 
 volatile bool echo2_flag = false; 

 volatile long duration;
 volatile long duration2;
  
 int64_t alarm_callback(alarm_id_t id, void *user_data) {
     alarm_flag = true;
     return 0;
 }

 int64_t alarm2_callback(alarm_id_t id, void *user_data) {
    alarm2_flag = true;
    return 0;
}

 
 void echo_callback(uint gpio, uint32_t events) {
     static uint32_t start_time;
     static uint32_t end_time;

     static uint32_t start2_time;
     static uint32_t end2_time;
    
    if (gpio == ECHO_PIN){
        if (events & GPIO_IRQ_EDGE_RISE) {
            start_time = time_us_32();
        } else if (events & GPIO_IRQ_EDGE_FALL) {
            end_time = time_us_32();
            duration = end_time - start_time;
            echo_flag = true;
        }
    }


    if (gpio == ECHO_PIN_2){
        if (events & GPIO_IRQ_EDGE_RISE) {
            start2_time = time_us_32();
        } else if (events & GPIO_IRQ_EDGE_FALL) {
            end2_time = time_us_32();
            duration2 = end2_time - start2_time;
            echo2_flag = true;
        }
    }

 }


 
 int main() {
     stdio_init_all();

      
     gpio_init(TRIG_PIN_2);
     gpio_set_dir(TRIG_PIN_2, GPIO_OUT);
     gpio_put(TRIG_PIN_2, 0);
 
     gpio_init(ECHO_PIN_2);
     gpio_set_dir(ECHO_PIN_2, GPIO_IN);
 
     gpio_init(TRIG_PIN);
     gpio_set_dir(TRIG_PIN, GPIO_OUT);
     gpio_put(TRIG_PIN, 0);
 
     gpio_init(ECHO_PIN);
     gpio_set_dir(ECHO_PIN, GPIO_IN);
     
     gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_callback);
 
     gpio_set_irq_enabled_with_callback(ECHO_PIN_2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_callback);
 
     while (true) {
        sleep_ms(100);
    
        // Reset flags e cancelar alarmes pendentes
        alarm_flag = false;
        alarm2_flag = false;
        echo_flag = false;
        echo2_flag = false;
    
        gpio_put(TRIG_PIN, 1);
        gpio_put(TRIG_PIN_2, 1);
        sleep_us(10);
        gpio_put(TRIG_PIN, 0);
        gpio_put(TRIG_PIN_2, 0);
    
        // Definindo os alarmes
        alarm_id_t alarm = add_alarm_in_us(50000, alarm_callback, NULL, true);
        alarm_id_t alarm2 = add_alarm_in_us(50000, alarm2_callback, NULL, true);
    
        sleep_ms(60); // Aguarda tempo suficiente para o retorno dos sensores
    
        if (echo_flag) {
            int distancia = (int)((duration * 0.0343) / 2.0);
            printf("Sensor 1 - dist: %d cm\n", distancia);
        } else if (alarm_flag) {
            printf("Sensor 1 - dist: falha\n");
        }
        cancel_alarm(alarm);
    
        if (echo2_flag) {
            int distancia2 = (int)((duration2 * 0.0343) / 2.0);
            printf("Sensor 2 - dist: %d cm\n", distancia2);
        } else if (alarm2_flag) {
            printf("Sensor 2 - dist: falha\n");
        }
        cancel_alarm(alarm2);
    }
    
 }