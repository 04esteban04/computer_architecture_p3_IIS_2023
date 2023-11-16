#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/ledc.h"
//#include "dht11.h"


// Definiciones de pines
//#define MOTOR1_ENABLE   GPIO_NUM_1
#define MOTOR1_A        GPIO_NUM_8
#define MOTOR1_B        GPIO_NUM_10

#define MOTOR2_ENABLE   GPIO_NUM_3
#define MOTOR2_A        GPIO_NUM_2
#define MOTOR2_B        GPIO_NUM_11


void forward(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 1);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 1);
    gpio_set_level(motor2_B, 0);
}

void app_main(void) {
    // Motor 1
    //gpio_set_direction(MOTOR1_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR1_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR1_B, GPIO_MODE_OUTPUT);

    //gpio_set_pull_mode(MOTOR1_ENABLE, GPIO_PULLUP_ENABLE);
    gpio_set_pull_mode(MOTOR1_A, GPIO_PULLUP_ENABLE);
    gpio_set_pull_mode(MOTOR1_B, GPIO_PULLUP_ENABLE);
    
    //gpio_set_level(MOTOR1_ENABLE, 1);
    gpio_set_level(MOTOR1_A, 1);
    gpio_set_level(MOTOR1_B, 1);

    // Motor 2
    gpio_set_direction(MOTOR2_ENABLE, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR2_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR2_B, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(MOTOR2_ENABLE, GPIO_PULLUP_ENABLE);
    gpio_set_pull_mode(MOTOR2_A, GPIO_PULLUP_ENABLE);
    gpio_set_pull_mode(MOTOR2_B, GPIO_PULLUP_ENABLE);

    gpio_set_level(MOTOR2_ENABLE, 1);
    gpio_set_level(MOTOR2_A, 1);
    gpio_set_level(MOTOR2_B, 1);

    //DHT11_init(GPIO_NUM_1);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = 1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0
    };

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT, // Resolución de 8 bits
        .freq_hz = 5000,                      // Frecuencia de 5000 Hz
        .speed_mode = LEDC_LOW_SPEED_MODE,   // Modo de velocidad
        .timer_num = LEDC_TIMER_0,            // Temporizador 0
        .clk_cfg = LEDC_AUTO_CLK,             // Configuración automática del reloj
    };

    // Configura el temporizador y el canal LEDC
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);

    // Establece un ciclo de trabajo del 25% (64/255) en el canal 0
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 128);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);


    while (true) {
        //forward(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);

        // Motor 1
        gpio_set_level(MOTOR1_A, 1);
        gpio_set_level(MOTOR1_B, 0);
        //gpio_set_level(MOTOR1_ENABLE, 1);
        // Motor 2
        gpio_set_level(MOTOR2_A, 1);
        gpio_set_level(MOTOR2_B, 0);
        gpio_set_level(MOTOR2_ENABLE, 1);
        vTaskDelay(300);
        
        // Motor 1
        gpio_set_level(MOTOR1_A, 0);
        gpio_set_level(MOTOR1_B, 0);
        //gpio_set_level(MOTOR1_ENABLE, 0);
        // Motor 2
        gpio_set_level(MOTOR2_A, 0);
        gpio_set_level(MOTOR2_B, 0);
        gpio_set_level(MOTOR2_ENABLE, 0);
        vTaskDelay(100);

        /*
        ESP_LOGI("", "GPIO 23: level: %d", gpio_get_level(MOTOR1_A));
        ESP_LOGI("", "GPIO 22: level: %d", gpio_get_level(MOTOR1_B));
        ESP_LOGI("", "GPIO 20: level: %d", gpio_get_level(MOTOR2_A));
        ESP_LOGI("", "GPIO 19: level: %d", gpio_get_level(MOTOR2_B));
        ESP_LOGI("---", "---");

        vTaskDelay(100);
        */
    }
}
