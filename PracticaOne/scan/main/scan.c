#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "myUart.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* ----- Utils -----*/
#define LED_GPIO1 5
#define LED_GPIO2 18
#define LED_GPIO3 19
#define EXAMPLE_ESP_MAXIMUM_RETRY  2
#define DEFAULT_SCAN_LIST_SIZE 10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

int rssiAverage = 0;
uint8_t ledstate = 0;
char pass[32];
char ssid[32];
int nred;
char ssidSelected[32];
char passwordSelected[64];

int RETRY = 2;
int i,j,k;
int rssiScan = 0;

static const char *TAG = "scan";


static EventGroupHandle_t s_wifi_event_group;

static const char *TAG_STATION = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_STATION, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_STATION,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_STATION, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta()
{
    
    s_wifi_event_group = xEventGroupCreate();
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));


    wifi_config_t wifi_config = {
        .sta = {
	     .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    memcpy(wifi_config.sta.ssid,ssidSelected,32);
    memcpy(wifi_config.sta.password,passwordSelected,64);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    RETRY = 2;
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG_STATION, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STATION, "connected to ap SSID:%s password:%s",
                ssidSelected, passwordSelected);
        if (abs(rssiAverage) < 50){
            gpio_set_level(LED_GPIO1, !ledstate);
            gpio_set_level(LED_GPIO2, !ledstate);
            gpio_set_level(LED_GPIO3, !ledstate);
        }else if(abs(rssiAverage) > 50 && abs(rssiAverage) < 70){
            gpio_set_level(LED_GPIO1, !ledstate);
            gpio_set_level(LED_GPIO2, !ledstate);
        }else if(abs(rssiAverage) > 70){
            gpio_set_level(LED_GPIO1, !ledstate);
        }
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STATION, "Failed to connect to SSID:%s, password:%s",
                 ssidSelected, passwordSelected);
    } else {
        ESP_LOGE(TAG_STATION, "UNEXPECTED EVENT");
    }
}


static void print_auth_mode(int authmode)
{
    switch (authmode) {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}


/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    printf("\n\n");

    printf(" ----- LISTA DE REDES -----\n");
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    for (i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        printf("Red número %d\n", i+1);
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        printf("\n");
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    esp_wifi_scan_stop();

    uartGetchar(0);
    uartSetColor(0,YELLOW);
    uartPuts(0,"Introduce la red deseada:"); 

    uartSetColor(0,BLUE);
    uartGets(0,pass);
    nred = myAtoi(pass) - 1;
    strcpy(ssidSelected,(char*)ap_info[nred].ssid);

    printf("\nObteniendo RSSI promedio...\n");
    rssiScan = 0;
    for(j = 0; j < 5; j++){ 
        esp_wifi_scan_start(NULL, true);
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        for (k = 0; (k < DEFAULT_SCAN_LIST_SIZE) && (k < ap_count); k++) {
            if(strcmp(ssidSelected,(char*)ap_info[k].ssid) == 0){
                rssiScan++;
                rssiAverage = rssiAverage + ap_info[k].rssi;
            }
        }
        esp_wifi_scan_stop();
    }
    if (rssiScan == 0){rssiScan++;}
    rssiAverage = rssiAverage / rssiScan;
    printf(" RSSI promedio de %s: %d\n", ssid,rssiAverage);

    uartSetColor(0,GREEN);
    uartPuts(0,"Introduce la contraseña:");
    uartGets(0,passwordSelected);
    printf("\n");

    wifi_init_sta();

    uartGetchar(0);
    uartSetColor(0,YELLOW);
    uartClrScr(0); 
    uartPuts(0,"Presione para conectarse a otra red"); 
    uartGets(0,pass);
    RETRY = -1;
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    gpio_set_level(LED_GPIO1, ledstate);
    gpio_set_level(LED_GPIO2, ledstate);
    gpio_set_level(LED_GPIO3, ledstate);

}

void app_main(void)
{
    // GPIO initialization
    gpio_set_direction(LED_GPIO1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO3, GPIO_MODE_OUTPUT);

    // UART terminal connection
    uartGetchar(PC_UART_PORT);
    uartInit(PC_UART_PORT, PC_UART_TX_PIN, PC_UART_RX_PIN);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Scan event
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    while(1){
        wifi_scan();
        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
    
}

