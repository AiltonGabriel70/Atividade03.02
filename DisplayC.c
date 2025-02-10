#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "pico/stdio.h"
#include "ws2812.pio.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/uart.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define BOTAO_A 5
#define BOTAO_B 6
#define LED_VERDE 11
#define LED_AZUL 12
#define NUM_WS2812 25
#define WS2812_PIN 7



// Função para converter valores RGB em um formato de 32 bits para o LED
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return (g << 16) | (r << 8) | b;
}

// Função para enviar um pixel de cor para o LED
void put_pixel(uint32_t color) {
    pio_sm_put_blocking(pio0, 0, color << 8u);
}

// Função para exibir a palavra "FLAMENGO" com os números de 0 a 9
void set_leds_for_char(char character) {
    const bool letters[10][NUM_WS2812] = {
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}, // F
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // L
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1}, // A
        {1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1}, // M
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // E
        {1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1}, // N
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // G
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // O
        {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0}, // Coração
        {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}  // !
    };

    int letter_index = -1;

    if (character == '0') letter_index = 0; // F
    if (character == '1') letter_index = 1; // L
    if (character == '2') letter_index = 2; // A
    if (character == '3') letter_index = 3; // M
    if (character == '4') letter_index = 4; // E
    if (character == '5') letter_index = 5; // N
    if (character == '6') letter_index = 6; // G
    if (character == '7') letter_index = 7; // O
    if (character == '8') letter_index = 8; // coração
    if (character == '9') letter_index = 9; // !

    if (letter_index >= 0 && letter_index < 10) {
        for (int i = 0; i < NUM_WS2812; i++) {
            if (letters[letter_index][NUM_WS2812 - 1 - i]) {
                put_pixel(urgb_u32(10, 0, 0)); // LED vermelho
            } else {
                put_pixel(0);  // Desliga o LED
            }
        }
    }
}

volatile bool botao_A_estado = true;
volatile bool botao_B_estado = false;
uint32_t last_button_press_time_A = 0;
uint32_t last_button_press_time_B = 0;
#define DEBOUNCE_DELAY 300  // Aumentamos o debounce para 300ms

ssd1306_t ssd;
int led_verde_estado = 0; // 0: Desligado, 1: Ligado
int led_azul_estado = 0;  // 0: Desligado, 1: Ligado

// Função de interrupção do Botão A
void botao_A_isr(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_button_press_time_A > DEBOUNCE_DELAY) {
        botao_A_estado = true;
        last_button_press_time_A = current_time;
    }
}

// Função de interrupção do Botão B
void botao_B_isr(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_button_press_time_B > DEBOUNCE_DELAY) {
        botao_B_estado = true;
        last_button_press_time_B = current_time;
    }
}

// Inicializa o display SSD1306
void init_display() {
    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Atualiza a mensagem no display
void atualiza_display(char* mensagem) {
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, mensagem, 10, 10);
    ssd1306_send_data(&ssd);
}

// Função que altera o estado do LED Verde
void alterar_estado_led_verde() {
    led_verde_estado = !led_verde_estado;
    if (led_verde_estado) {
        gpio_put(LED_VERDE, 1); // Acende o LED verde
        atualiza_display("LED Verde ON");
        printf("LED Verde ON\n");
    } else {
        gpio_put(LED_VERDE, 0); // Apaga o LED verde
        atualiza_display("LED Verde OFF");
        printf("LED Verde OFF\n");
    }
}

// Função que altera o estado do LED Azul
void alterar_estado_led_azul() {
    led_azul_estado = !led_azul_estado;
    if (led_azul_estado) {
        gpio_put(LED_AZUL, 1); // Acende o LED azul
        atualiza_display("LED Azul ON");
        printf("LED Azul ON\n");
    } else {
        gpio_put(LED_AZUL, 0); // Apaga o LED azul
        atualiza_display("LED Azul OFF");
        printf("LED Azul OFF\n");
    }
}

// Configuração das interrupções dos botões
void setup_button_interrupts() {
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &botao_A_isr);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &botao_B_isr);
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);

    // Inicializa o PIO para os LEDs WS2812
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, false);

    init_display();
    atualiza_display("Aguardando...");

    printf("Sistema inicializado!\n");

    // Configuração das interrupções
    setup_button_interrupts();

    // Inicializa a UART
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    while (1) {
        if (botao_A_estado) {
            botao_A_estado = false;
            alterar_estado_led_verde();  // O botão A aciona o LED verde
        }

        if (botao_B_estado) {
            botao_B_estado = false;
            alterar_estado_led_azul();  // O botão B aciona o LED azul
        }

        // Verifica se a comunicação serial está conectada antes de tentar ler
        if (stdio_usb_connected()) {
            int c = getchar_timeout_us(0);  // Lê o caractere ASCII da entrada serial
            if (c != PICO_ERROR_TIMEOUT) {
                if (c >= '0' && c <= '9') {  // Verifica se o número está entre 0 e 9
                    // Exibe o caractere correspondente nos LEDs
                    set_leds_for_char(c);  // Exibe o caractere nos LEDs
                    const char letra[2] = {c, '\0'};
                    atualiza_display((char*)letra);  // Atualiza o display com a letra/número correspondente
                    printf("Caractere recebido: %c\n", c);  // Exibe no terminal
                }
            }
        } else {
            sleep_ms(100);  // Aguarda a conexão do USB
        }
    }
}
