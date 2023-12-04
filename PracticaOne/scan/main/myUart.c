#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "myUart.h"

// UART 0
#define PC_UART_PORT    (0)
#define PC_UART_RX_PIN  (3)
#define PC_UART_TX_PIN  (1)
// UART 1
#define UART1_PORT      (1)
#define UART1_RX_PIN    (18)
#define UART1_TX_PIN    (19)
// UART 2
#define UART2_PORT      (2)
#define UART2_RX_PIN    (16)
#define UART2_TX_PIN    (17)

#define UARTS_BAUD_RATE         (115200)
#define TASK_STACK_SIZE         (1048)
#define READ_BUF_SIZE           (1024)


/**
 * @brief Configure and install the default UART, then, connect it to the
 * console UART.
 */
void uartInit(uart_port_t uart_num, uint8_t txPin, uint8_t rxPin)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = _8BITS_SIZE,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
};
// Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_driver_install(uart_num, READ_BUF_SIZE, READ_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

}

//Funciones auxiliares
void swap(char *x, char *y) 
{
    char t = *x; *x = *y; *y = t;
}
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}
void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void uartClrScr(uart_port_t uart_num)
{
    // Uso "const" para sugerir que el contenido del arreglo lo coloque en Flash y no en RAM
    const char caClearScr[] = "\033[H\033[2J";
    uart_write_bytes(uart_num, caClearScr, sizeof(caClearScr));
}

void uartSetColor(uart_port_t uart_num, uint8_t color) //TERMINADA
{
     char casetColor[5] = {'\033','[','3','X','m'};
    switch (color)
    {
        case YELLOW:
        {
            casetColor[3] = '3';
            break;
        }
        case BLUE:
        {
            casetColor[3] = '4';
            break;
        }
        case GREEN:
        {
           casetColor[3] = '2';
            break;
        }
    }
    uart_write_bytes(uart_num, (const char*)casetColor, sizeof(casetColor));
}
    
uint16_t myAtoi(char *str) // TERMINADA
{
    uint16_t number = 0;
    uint16_t i = 0;
    while(str[i] >= '0' && str[i] <= '9')
    {
        number = number * 10 + str[i] - '0';
        i++;
    }
    return number;
}

void uartGotoxy(uart_port_t uart_num, uint8_t x, uint8_t y) //TERMINADA
{
    char caGotoxy[10] = {'\e','['};
    char cad2[3];
    uint8_t a = 2;
    uint8_t b = 0;
    while(x > 0)
    {
        cad2[b] = (char)(x % 10) + 48;
        b++;
        x/=10;
    }
    while(b > 0)
    {
        caGotoxy[a] = cad2[b-1];
        a++;
        b--;
    }
    caGotoxy[a] = ';';
    a++;
    while(y > 0)
    {
        cad2[b] = (char)(y % 10) + 48;
        b++;
        y/=10;
    }
    while(b > 0)
    {
        caGotoxy[a] = cad2[b-1];
        a++;
        b--;
    }
    caGotoxy[a] = 'H';
    uart_write_bytes(uart_num, caGotoxy, sizeof(caGotoxy));
}

void myItoa(uint16_t number, char* str, uint8_t base) //TERMINADA
{
    uint16_t i = 0;  
    while(number)
    {
        uint16_t r = number % base;
        if (r >= 0 && r <= 9) {
            str[i] = '0' + r;
        }
        else {
            str[i] = 'A' + (r - 10);
        }
        i++;
        number = number / base;
    }
    str[i] = '\0';
    reverse(str, 0, i - 1);
}

bool uartKbhit(uart_port_t uart_num)
{
    uint8_t length;
    uart_get_buffered_data_len(uart_num, (size_t*)&length);
    return (length > 0);
}

void uartPutchar(uart_port_t uart_num, char c)
{
    uart_write_bytes(uart_num, &c, sizeof(c));
}

void uartPuts(uart_port_t uart_num, char *str)
{ 
    while (*str)
    {
        uartPutchar(uart_num , *str);
        str++;
    }
}

char uartGetchar(uart_port_t uart_num)
{
    char c;
    // Wait for a received byte
    while(!uartKbhit(uart_num))
    {
        delayMs(10);
    }
    // read byte, no wait
    uart_read_bytes(uart_num, &c, sizeof(c), 0);
    return c;
}

void uartGets(uart_port_t uart_num, char *str)
{
    uint8_t i = 0;
    char *tmp = str;
    char c;
    do
    {
        c = uartGetchar(uart_num);
        if (c != '\r')
        {
            if(c != 8)
            {
                *str = c;
                str++;
                i++;
                uartPutchar(uart_num, c);
            }
            else 
            {
                if ( str > tmp)
                {
                    str--;
                    uartPutchar(uart_num, 8);
                    uartPutchar(uart_num, 32);
                    uartPutchar(uart_num, 8);
                }
            }
        }    
    } while (c != '\r');
    *str = '\0';
}
