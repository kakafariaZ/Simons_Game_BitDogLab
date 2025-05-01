#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>

// Definições de GPIOs
#define LED_R 13
#define LED_G 11
#define LED_B 12

#define BUT_A 5
#define BUT_B 6
#define BUT_JOY 22

#define BUZZER_A 10

// Notas musicais
#define NOTA_LA 440
#define NOTA_SI 494
#define NOTA_DO 523

// Tamanho máximo da sequência
#define MAX_SEQ 100

// Variáveis globais
uint8_t sequencia[MAX_SEQ];
uint8_t fase = 1;

// Função para iniciar PWM no buzzer
void buzzer_init() {
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A);
    pwm_set_enabled(slice_num, true);
}

// Tocar nota por X ms
void tocar_nota(int freq, int duracao_ms) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A);

    pwm_set_wrap(slice_num, 125000000 / freq);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, (125000000 / freq) / 2); // 50% duty cycle

    sleep_ms(duracao_ms);

    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    sleep_ms(100);
}

// Mostrar sequência de LEDs + som
void mostrar_sequencia(uint8_t *seq, uint8_t tamanho) {
    printf("Sequência gerada: ");
    for (int i = 0; i < tamanho; i++) {
        printf("%d ", seq[i]);
    }
    printf("\n");

    for (int i = 0; i < tamanho; i++) {
        if (seq[i] == 0) { // Vermelho
            gpio_put(LED_R, 1);
            tocar_nota(NOTA_LA, 300);
            gpio_put(LED_R, 0);
        } else if (seq[i] == 1) { // Verde
            gpio_put(LED_G, 1);
            tocar_nota(NOTA_SI, 300);
            gpio_put(LED_G, 0);
        } else if (seq[i] == 2) { // Azul
            gpio_put(LED_B, 1);
            tocar_nota(NOTA_DO, 300);
            gpio_put(LED_B, 0);
        }
        sleep_ms(200);
    }
}


// Função de mini debounce para botões
bool botao_pressionado(uint gpio) {
    if (gpio_get(gpio)) {
        sleep_ms(20); // Debounce
        if (gpio_get(gpio)) {
            while (gpio_get(gpio)) {
                sleep_ms(1); // Não travar totalmente
            }
            return true;
        }
    }
    return false;
}

// Ler entrada do jogador
void ler_entrada(uint8_t *entrada, uint8_t tamanho) {
    int contador = 0;
    while (contador < tamanho) {
        printf("Aguardando entrada (contador: %d/%d)...\n", contador, tamanho);
        
        // Debug dos estados dos botões
        printf("Estados - A: %d, B: %d, Joy: %d\n", 
               gpio_get(BUT_A), gpio_get(BUT_B), gpio_get(BUT_JOY));
        
        if (botao_pressionado(BUT_A) == 0) {
            printf("Botão A pressionado\n");
            entrada[contador++] = 0;
            gpio_put(LED_R, 1);
            tocar_nota(NOTA_LA, 200);
            gpio_put(LED_R, 0);
        } 
        else if (botao_pressionado(BUT_B) == 0) {
            printf("Botão B pressionado\n");
            entrada[contador++] = 1;
            gpio_put(LED_G, 1);
            tocar_nota(NOTA_SI, 200);
            gpio_put(LED_G, 0);
        }
        else if (botao_pressionado(BUT_JOY) == 0) {
            printf("Botão Joy pressionado\n");
            entrada[contador++] = 2;
            gpio_put(LED_B, 1);
            tocar_nota(NOTA_DO, 200);
            gpio_put(LED_B, 0);
        }
        
        sleep_ms(50); // Aumentei o tempo para facilitar o debug
    }
}

void piscar_todos(int vezes) {
    for (int i = 0; i < vezes; i++) {
        gpio_put(LED_R, 1);
        gpio_put(LED_G, 1);
        gpio_put(LED_B, 1);
        sleep_ms(200);
        gpio_put(LED_R, 0);
        gpio_put(LED_G, 0);
        gpio_put(LED_B, 0);
        sleep_ms(200);
    }
}

int main() {
    stdio_init_all();

    // Inicializar GPIOs
    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);

    printf("Testando configuração de botões...\n");
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_init(BUT_A);
    gpio_init(BUT_B);
    gpio_init(BUT_JOY);
    gpio_set_dir(BUT_A, GPIO_IN);
    gpio_set_dir(BUT_B, GPIO_IN);
    gpio_set_dir(BUT_JOY, GPIO_IN);
    gpio_pull_up(BUT_A);
    gpio_pull_up(BUT_B);
    gpio_pull_up(BUT_JOY);

    buzzer_init();

    // Gerar uma "seed" improvisada
    sleep_ms(1000); // Espera um pouco ao ligar (tempo variável)
    srand((uint32_t)time_us_32());

    while (1) {
        printf("Fase %d\n", fase);
        sequencia[fase - 1] = rand() % 3; // Adiciona nova cor
        mostrar_sequencia(sequencia, fase);

        uint8_t entrada[MAX_SEQ] = {0};
        ler_entrada(entrada, fase);

        int erro = 0;
        for (int i = 0; i < fase; i++) {
            if (entrada[i] != sequencia[i]) {
                erro = 1;
                break;
            }
        }

        if (!erro) {
            printf("Acertou!\n");
            fase++;
            sleep_ms(1000);
        } else {
            printf("Errou! Fim de jogo.\n");
            piscar_todos(3);
            fase = 1; // Reinicia o jogo
            sleep_ms(1000);
        }
    }
}
