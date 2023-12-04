/*Codigo terminado donde en una pagina web se muestra la temperatura en tiempo real
en otra pagina se muestran las 10 ultimas lecturas*/


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <esp_http_server.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define EXAMPLE_ESP_WIFI_SSID      "MarioPractica"
#define EXAMPLE_ESP_WIFI_PASS      "87654321"
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define ADC1_EXAMPLE_CHAN0          ADC1_CHANNEL_6
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF


static const char *TAG = "wifi softAP";
static int adc_raw[10];
static esp_adc_cal_characteristics_t adc1_chars;
uint32_t voltage = 0;
uint32_t  voltajes[10];
char buffer[10][10];
static int i = 0;
bool cali_enable;

static esp_err_t commands_handler(httpd_req_t *req){

    adc_raw[0] = adc1_get_raw(ADC1_EXAMPLE_CHAN0);
        if (cali_enable) {
            voltage = esp_adc_cal_raw_to_voltage(adc_raw[0], &adc1_chars);
        }
    voltajes[i] = voltage;
    i++;
    i = i%10;
    
    char buffer1[100];
    snprintf(buffer1, sizeof(buffer1), "%ld", voltage);
    const char* sensor1_str = buffer1;  

        const char* html_template = "<!DOCTYPE html>\n"
                                "<html>\n"
                                "<head>\n"
                                "  <style>\n"
                                "    /* Estilos CSS adicionales */\n"
                                "    body {\n"
                                "      font-family: sans-serif;\n"
                                "      background-color: black;\n"
                                "      display: flex;\n"
                                "      justify-content: center;\n"
                                "      align-items: center;\n"
                                "      height: 100vh;\n"
                                "      margin: 0;\n"
                                "      flex-direction: column; /* Añadido para alinear verticalmente */\n"
                                "    }\n"
                                "    h1 {\n"
                                "      font-family: sans-serif;\n"
                                "      color: white;\n"
                                "    }\n"
                                "    .container {\n"
                                "      display: flex;\n"
                                "      justify-content: center;\n"
                                "      align-items: center;\n"
                                "      gap: 10px;\n"
                                "    }\n"
                                "    .mi-div {\n"
                                "      font-family: sans-serif;\n"
                                "      background-color: #565656;\n"
                                "      color: white;\n"
                                "      font-size: 18px;\n"
                                "      padding: 10px;\n"
                                "      border-radius: 50vw;\n"
                                "      width: 200px;\n"
                                "      height: 200px;\n"
                                "      display: flex;\n"
                                "      justify-content: center;\n"
                                "      align-items: center;\n"
                                "      flex-direction: column; /* Añadido para alinear verticalmente */\n"
                                "    }\n"
                                "    .subtitle {\n"
                                "      font-size: 14px;\n"
                                "      margin-top: 5px;\n"
                                "    }\n"
                                "    .value {\n"
                                "      margin-bottom: 5px;\n"
                                "    }\n"
                                "  </style>\n"
                                "    <script>\n"
                                "        setInterval(updateValues, 3000);\n"
                                "        function updateValues() {\n"
                                "            location.reload(); \n"
                                "        }\n"
                                "    </script>\n"
                                "</head>\n"
                                "<body>\n"
                                "  <h1>SENSADO</h1>\n"
                                "  <div class=\"container\">\n"
                                "    <div class=\"mi-div\">\n"
                                "      <div style=\"background-color: #343434; width: 150px; height: 150px; border-radius: 50vw; display: flex; justify-content: center; align-items: center; flex-direction: column;\">\n"
                                "          <div class=\"value\">%s</div>\n"
                                "        <div class=\"subtitle\">Temperatura</div>\n"
                                "      </div>\n"
                                "    </div>\n"
                                "  </div>\n"
                                " <form action=\"/historial\" method=\"post\">\n"
                                " <button name=\"Ver historial\" value=\"0x11\">Ver historial</button>\n"
                                "</form>\n"
                                "</body>\n"
                                "</html>";
    size_t html_size = snprintf(NULL, 0, html_template, sensor1_str) + 1;

    char* html_response = malloc(html_size);
    snprintf(html_response, html_size, html_template, sensor1_str);

    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
    
    // Liberar la memoria asignada para la respuesta HTML
    free(html_response);
    return ESP_OK;
}

static esp_err_t ver_historial_handler(httpd_req_t *req){

    int j = 0;

    while (j < 10)
    {
        snprintf(buffer[j], sizeof(buffer[j]), "%ld", voltajes[j]);
        j++;
    }

    const char* sensor0_str = buffer[0];
    const char* sensor1_str = buffer[1];
    const char* sensor2_str = buffer[2];
    const char* sensor3_str = buffer[3];
    const char* sensor4_str = buffer[4];
    const char* sensor5_str = buffer[5];
    const char* sensor6_str = buffer[6];
    const char* sensor7_str = buffer[7];
    const char* sensor8_str = buffer[8];
    const char* sensor9_str = buffer[9];


    const char* html_template = "<h1>Lecturas</h1>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                "%s<br>\n"
                                " <form action=\"/paginaweb\" method=\"get\">\n"
                                " <button name=\"Ver humaedad\" value=\"0x11\">Humedad tiempo real</button>\n"
                                "</form>\n";
        

    size_t html_size = snprintf(NULL, 0, html_template, sensor0_str, sensor1_str, sensor2_str, sensor3_str ,sensor4_str, sensor5_str, sensor6_str, sensor7_str, sensor8_str ,sensor9_str) + 1;

    char* html_response = malloc(html_size);
    snprintf(html_response, html_size, html_template,  sensor0_str, sensor1_str, sensor2_str, sensor3_str ,sensor4_str, sensor5_str, sensor6_str, sensor7_str, sensor8_str ,sensor9_str);

    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
    
    // Liberar la memoria asignada para la respuesta HTML
    free(html_response);
    return ESP_OK;

}

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}


static const httpd_uri_t commands = {
    .uri      = "/paginaweb",
    .method   = HTTP_GET,
    .handler  = commands_handler,
    .user_ctx = NULL
};

static const httpd_uri_t historial = {
    .uri      = "/historial",
    .method   = HTTP_POST,
    .handler  = ver_historial_handler,
    .user_ctx = NULL
};

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Iniciar el servidor httpd 
    ESP_LOGI(TAG, "Iniciando el servidor en el puerto: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Manejadores de URI
        ESP_LOGI(TAG, "Registrando manejadores de URI");
        httpd_register_uri_handler(server, &commands);
        httpd_register_uri_handler(server, &historial);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void app_main(void)
{
    //Initialize NVS
    cali_enable = adc_calibration_init();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));
        ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();
    httpd_handle_t server = NULL;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    server = start_webserver();
}
