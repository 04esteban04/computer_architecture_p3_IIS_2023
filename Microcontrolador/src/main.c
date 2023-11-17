#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht11.h"

// Definiciones de pines
#define MOTOR1_A    GPIO_NUM_8
#define MOTOR1_B    GPIO_NUM_10

#define MOTOR2_A    GPIO_NUM_2
#define MOTOR2_B    GPIO_NUM_11

int contador = 1;
int valorIR = 0;

void moverAdelante(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 1);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 1);
    gpio_set_level(motor2_B, 0);
}

void moverAtras(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 1);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 1);
}

void moverDerecha(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 1);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 0);
}

void moverIzquierda(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 1);
    gpio_set_level(motor2_B, 0);
}

void detener(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 0);
}

void corregirMovimiento(){
    moverDerecha(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(30);
    detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(10);
    
    moverAtras(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(30);
    detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(10);
    
    moverIzquierda(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(30);
    detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(10);

    moverAtras(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(10);
    detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(30);
}

void leerSensorDIH(){
    //Sensor humedad
    if (DHT11_read().status == DHT11_OK) { // Verifica si la lectura fue exitosa
        ESP_LOGI("DHT11", "Temperatura: %d°C, Humedad: %d%% \n", DHT11_read().temperature, DHT11_read().humidity); 
    } else { 
        ESP_LOGE("DHT11", "Error al leer el sensor: %d\n", DHT11_read().status);
    }
}


void app_main(void) {

    // Inicialización de pines y valores
    // Motor 1
    gpio_set_direction(MOTOR1_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR1_B, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR1_A, 0);
    gpio_set_level(MOTOR1_B, 0);

    // Motor 2
    gpio_set_direction(MOTOR2_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR2_B, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR2_A, 0);
    gpio_set_level(MOTOR2_B, 0);

    // Sensor de humedad
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    DHT11_init(GPIO_NUM_0);

    // Sensor infrarrojo
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);

    // Ejecución del loop
    while (true) {

        // Sensor IR
        valorIR = gpio_get_level(GPIO_NUM_1);
        ESP_LOGI("Lectura", "%d -- IR : %d", contador, valorIR);

        // Sensor Humedad
        leerSensorDIH();
        
        // Se continua en el camino
        if (valorIR == 1) { 
            moverAdelante(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
            vTaskDelay(10);
            detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
            vTaskDelay(30);
        }

        // Se sale del camino
        else if (valorIR == 0) {
            corregirMovimiento();
        }

        contador++; 
        
    }

}

