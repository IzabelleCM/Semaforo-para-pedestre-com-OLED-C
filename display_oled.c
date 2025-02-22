#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

// Define os pinos do LED
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Define o pino do botão
#define BTN_A_PIN 5

// Define o SDA e SCL do I2C para o display OLED
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// Estado do botão
int A_state = 0;

// Buffer do display OLED
uint8_t ssd[ssd1306_buffer_length];

// Área de renderização do display OLED
struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};

// Mensagens para o display
char *text1[] = {
    "SINAL ABERTO",    //Aberto para o pedestre e fechado para os carros
    "-",
    "ATRAVESSAR COM",
    "CUIDADO"
};
char *text2[] = {
    "SINAL DE",        //Sinal de atenção para os dois
    "ATENCAO",
    "-",
    "PREPARE-SE"
};
char *text3[] = {
    "SINAL FECHADO",   //Fechado para os pedestres e aberto para os carros
    "-",
    "AGUARDE",
    ""
};

// Função para exibir mensagens no display
void show_message(char *text[]) {
    memset(ssd, 0, ssd1306_buffer_length);  // Limpa o display
    for (int i = 0; i < 4; i++) {
        ssd1306_draw_string(ssd, 5, i * 8, text[i]);  // Desenha cada linha
    }
    render_on_display(ssd, &frame_area);  // Renderiza no display
}

// Funções para controle do semáforo
void SinalAberto() {       // Sinal aberto (verde) para o pedestre
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    show_message(text1);   // Exibe mensagem correspondente
}

void SinalAtencao() {      // Sinal de atenção (amarelo) para os dois
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    show_message(text2);   // Exibe mensagem correspondente
}

void SinalFechado() {      // Sinal fechado (vermelho) para o pedestre
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    show_message(text3);  // Exibe mensagem correspondente
}

// Função para aguardar e verificar o botão
int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i += 100) {
        A_state = !gpio_get(BTN_A_PIN);  // Verifica o estado do botão
        if (A_state == 1) {
            return 1;  // Botão pressionado
        }
        sleep_ms(100);  // Espera 100ms
    }
    return 0;  // Tempo esgotado sem interação
}

int main() {
    // Inicializa LEDs
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Inicializa botão
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    // Inicializa comunicação serial
    stdio_init_all();

    // Inicializa I2C e display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();

    // Calcula o buffer da área de renderização
    calculate_render_area_buffer_length(&frame_area);
    memset(ssd, 0, ssd1306_buffer_length);  // Limpa o display
    render_on_display(ssd, &frame_area);    // Renderiza o display vazio

    while (true) {
        // Estado inicial: Sinal fechado (vermelho) para o pedestre
        SinalFechado();
        A_state = WaitWithRead(8000);  // Espera até 8 segundos ou interação com o botão

        if (A_state) {  // Botão pressionado
            SinalAtencao();  // Sinal de atenção (amarelo) para os dois
            sleep_ms(2000);  // Aguarda 2 segundos

            SinalAberto();  // Sinal aberto (verde) para o pedestre
            sleep_ms(10000); // Aguarda 10 segundos
        } else {  // Sem interação com o botão
            SinalAtencao();  // Sinal de atenção (amarelo)
            sleep_ms(2000);  // Aguarda 2 segundos

            SinalAberto();  // Sinal aberto (vermelho) para o pedestre
            sleep_ms(8000);  // Aguarda 8 segundos
        }
    }

    return 0;
}