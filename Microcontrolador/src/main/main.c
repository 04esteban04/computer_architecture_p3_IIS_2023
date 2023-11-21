#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "components/esp-dht11/dht11.c"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_pthread.h"


// *****************************************************
//            PINES Y VARIABLES DEL CARRO
// *****************************************************
#define MOTOR1_A    GPIO_NUM_8
#define MOTOR1_B    GPIO_NUM_10

#define MOTOR2_A    GPIO_NUM_2
#define MOTOR2_B    GPIO_NUM_11

int contadorLectura = 1;
int contadorMovimiento = 0;
int valorIR = 0;
char* resultadoSensorDIH;


// *****************************************************
//            VARIABLES DE CONEXION WIFI
// *****************************************************
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

// Socket
int sock;

// Grupo de eventos
static EventGroupHandle_t wifi_event_group;

// Cantidad de reconexiones
static int s_retry_num = 0;

// Tag de proceso
static const char *TAG = "WIFI";


// *****************************************************
//          FUNCIONES PARA EL MOVIMIENTO DEL CARRO
// *****************************************************
/*
Funcion que permite realizar el movimiento hacia adelante

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Genera el movimiento hacia adelante del robot
*/
void moverAdelante(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 1);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 1);
    gpio_set_level(motor2_B, 0);
}

/*
Funcion que permite realizar el movimiento hacia atras

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Genera el movimiento hacia atras del robot
*/
void moverAtras(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 1);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 1);
}

/*
Funcion que permite realizar el movimiento hacia la derecha

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Genera el movimiento hacia la derecha del robot
*/
void moverDerecha(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 1);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 0);
}

/*
Funcion que permite realizar el movimiento hacia la izquierda

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Genera el movimiento hacia la izquierda del robot
*/
void moverIzquierda(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 1);
    gpio_set_level(motor2_B, 0);
}

/*
Funcion que permite detener el movimiento del robot

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Detiene el movimiento del robot
*/
void detener(gpio_num_t motor1_A, gpio_num_t motor1_B, gpio_num_t motor2_A, gpio_num_t motor2_B) {
    // Motor 1
    gpio_set_level(motor1_A, 0);
    gpio_set_level(motor1_B, 0);

    // Motor 2
    gpio_set_level(motor2_A, 0);
    gpio_set_level(motor2_B, 0);
}

/*
Funcion que permite corregir el movimiento del robot

    Entradas:
        motor1_A: Pin A del motor 1
        motor1_A: Pin B del motor 1
        motor2_A: Pin A del motor 2
        motor2_A: Pin B del motor 2

    Salidas:
        Genera el movimiento corregido del robot
*/
void corregirMovimiento(){
    if (contadorMovimiento >= 16) {
        detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(30);
    }
    else if (contadorMovimiento > 5){    
        moverIzquierda(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(15);
        detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(30);
    }
    else if (contadorMovimiento == 5){
        moverAtras(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(30);
        detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(30);
    }
    else{
        moverDerecha(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(15);
        detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
        vTaskDelay(30);
    }

    contadorMovimiento++;

    /*
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
    vTaskDelay(30);
    detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
    vTaskDelay(10);
    */
}

/*
Funcion que permite realizar la lectura del sensor de humedad

    Entradas:
        N/A

    Salidas:
        Retorna la lectura del sensor de humedad como cadena de caracteres
*/
char* leerSensorDIH() {
    char* strData = (char*)malloc(100);

    if (DHT11_read().status == DHT11_OK) {
        sprintf(strData, "Temperatura: %d°C, Humedad: %d%% \n", DHT11_read().temperature, DHT11_read().humidity);
    } else {
        sprintf(strData, "Error al leer el sensor: %d\n", DHT11_read().status);
    }

    return strData;
}



// *****************************************************
//          FUNCIONES PARA LA CONEXION WIFI
// *****************************************************


// Manejador de eventos para wifi
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
		ESP_LOGI(TAG, "Connecting to AP...");
		esp_wifi_connect();
	} 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
		if (s_retry_num < MAX_FAILURES){
			ESP_LOGI(TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

// Manejador de eventos para ip
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

// Funcion para conectarse a wifi y devolver el resultado de la conexion
esp_err_t connect_wifi() {
	int status = WIFI_FAILURE;

	// Inicializar la interfaz de red del esp
	ESP_ERROR_CHECK(esp_netif_init());

	// Inicializar el ciclo de eventos del esp
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Crear la estacion wifi en el driver wifi
	esp_netif_create_default_wifi_sta();

	// Configurar la estacion wifi por defecto
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Manejador de eventos
	wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    // Inicializacion del driver wifi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ARRIS-3A53",
            .password = "50A5DC053A53",
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

    // Establecer el controlador wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Establecer la configuracion wifi
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Iniciar el driver wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    // Se espera a que ocurra un evento 
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_SUCCESS | WIFI_FAILURE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGI(TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    // Manejo de errores
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

// Funcion para conectarse al servidor y devolver el resultado
esp_err_t connect_tcp_server(void) {
	struct sockaddr_in serverInfo = {0};
	
    serverInfo.sin_family = AF_INET;
	//serverInfo.sin_addr.s_addr = 0x0100007f;
    serverInfo.sin_addr.s_addr = inet_addr("192.168.0.9");
	serverInfo.sin_port = htons(12345);


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		ESP_LOGE(TAG, "Failed to create a socket..?");
		return TCP_FAILURE;
	}

	if (connect(sock, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) != 0) {
		ESP_LOGE(TAG, "Failed to connect to %s!", inet_ntoa(serverInfo.sin_addr.s_addr));
		close(sock);
		return TCP_FAILURE;
	}

	ESP_LOGI(TAG, "Connected to TCP server.");
    return TCP_SUCCESS;
}

// Funcion para enviar datos al servidor
esp_err_t send_string_to_server(char *message) {

    if (send(sock, message, strlen(message), 0) < 0){
        ESP_LOGE(TAG, "Failed to send data to the server");
        //close(sock);
        return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "String sent to server: %s", message);
    return TCP_SUCCESS;
}

// Funcion para reconectarse al socket
esp_err_t reconnect(int sock, struct sockaddr_in *serverInfo) {
    ESP_LOGI(TAG, "Attempting to reconnect...");

    if (connect(sock, (struct sockaddr *)serverInfo, sizeof(*serverInfo)) != 0) {
        ESP_LOGE(TAG, "Failed to reconnect to %s!", inet_ntoa(serverInfo->sin_addr.s_addr));
        close(sock);
        return connect_tcp_server();
        //return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "Reconnected to TCP server.");

    return TCP_SUCCESS;
}

// Funcion auxiliar de reconexion
esp_err_t reconnect_tcp_server(const char* ip_addr_str) {
    struct sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr(ip_addr_str);
    serverInfo.sin_port = htons(12345); // Assuming your port number

    return reconnect(sock, &serverInfo);
}


// *****************************************************
//                          MAIN
// *****************************************************
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


    // Inicializacion de conexion wifi
    // Estado de conexion
    esp_err_t status = WIFI_FAILURE;
    esp_err_t sendDataStatus;

	// Inicializacion almacenamiento
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Conectarse a red wifi
    status = connect_wifi();
    if (WIFI_SUCCESS != status) {
        ESP_LOGI(TAG, "Failed to associate to AP, dying...");
        //return;
    }

    // Conectarse al servidor
    status = connect_tcp_server();
    if (TCP_SUCCESS != status) {
        ESP_LOGE(TAG, "Failed to connect to remote server, dying...");
    }

    // Ejecución del loop
    while (true) {

        // Sensor IR
        valorIR = gpio_get_level(GPIO_NUM_1);
        printf("Lectura %d -- IR : %d \n", contadorLectura, valorIR);

        // Sensor Humedad
        resultadoSensorDIH = leerSensorDIH();
        printf("Resultado del sensor: %s \n", resultadoSensorDIH);

        // Se continua en el camino
        if (valorIR == 1) { 
            moverAdelante(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
            vTaskDelay(10);
            detener(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B);
            vTaskDelay(30);
            contadorMovimiento = 0;
        }

        // Se sale del camino
        else if (valorIR == 0) {
            corregirMovimiento();
        }

        // Enviar el mensaje al servidor
        sendDataStatus = send_string_to_server(resultadoSensorDIH);        
        if (TCP_SUCCESS != sendDataStatus) {
            //ESP_LOGE(TAG, "Failed in remote server, dying...");
            status = reconnect_tcp_server("192.168.0.9");
        }
        
        contadorLectura++; 
    }
}
