#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/time.h"
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

// Debounce para interrupção
#define DEBOUNCE_TIME_US 200000
volatile uint32_t last_press_B = 0;
volatile bool flag_botao_B = false;

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
    pwm_set_chan_level(slice_num, PWM_CHAN_A, (125000000 / freq) / 2);
    sleep_ms(duracao_ms);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    sleep_ms(100);
}

// Mostrar sequência de LEDs + som
void mostrar_sequencia(uint8_t *seq, uint8_t tamanho) {
    for (int i = 0; i < tamanho; i++) {
        switch (seq[i]) {
            case 0:
                gpio_put(LED_R, 1);
                tocar_nota(NOTA_LA, 300);
                gpio_put(LED_R, 0);
                break;
            case 1:
                gpio_put(LED_G, 1);
                tocar_nota(NOTA_SI, 300);
                gpio_put(LED_G, 0);
                break;
            case 2:
                gpio_put(LED_B, 1);
                tocar_nota(NOTA_DO, 300);
                gpio_put(LED_B, 0);
                break;
        }
        sleep_ms(200);
    }
}

// Mini debounce para botões A e Joy (que ainda usam polling)
bool botao_pressionado(uint gpio) {
    if (!gpio_get(gpio)) {
        sleep_ms(20);
        if (!gpio_get(gpio)) {
            while (!gpio_get(gpio)) {
                sleep_ms(1);
            }
            return true;
        }
    }
    return false;
}

// Interrupção do botão B
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t agora = to_us_since_boot(get_absolute_time());
    if ((gpio == BUT_B) && (events & GPIO_IRQ_EDGE_FALL)) {
        if (agora - last_press_B > DEBOUNCE_TIME_US) {
            last_press_B = agora;
            flag_botao_B = true;  // Marca que o botão B foi pressionado
        }
    }
}

// Ler entrada do jogador
void ler_entrada(uint8_t *entrada, uint8_t tamanho) {
    int contador = 0;
    while (contador < tamanho) {
        if (botao_pressionado(BUT_A)) {
            entrada[contador++] = 0;
            gpio_put(LED_R, 1);
            tocar_nota(NOTA_LA, 200);
            gpio_put(LED_R, 0);
        } else if (flag_botao_B) {
            flag_botao_B = false;
            entrada[contador++] = 1;
            gpio_put(LED_G, 1);
            tocar_nota(NOTA_SI, 200);
            gpio_put(LED_G, 0);
        } else if (botao_pressionado(BUT_JOY)) {
            entrada[contador++] = 2;
            gpio_put(LED_B, 1);
            tocar_nota(NOTA_DO, 200);
            gpio_put(LED_B, 0);
        }
        sleep_ms(50);
    }
}

// Piscar todos os LEDs
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

    gpio_init(LED_R); gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G); gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B); gpio_set_dir(LED_B, GPIO_OUT);

    gpio_init(BUT_A); gpio_set_dir(BUT_A, GPIO_IN); gpio_pull_up(BUT_A);
    gpio_init(BUT_B); gpio_set_dir(BUT_B, GPIO_IN); gpio_pull_up(BUT_B);
    gpio_init(BUT_JOY); gpio_set_dir(BUT_JOY, GPIO_IN); gpio_pull_up(BUT_JOY);

    buzzer_init();

    // Configura interrupção para o botão B
    gpio_set_irq_enabled_with_callback(BUT_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    sleep_ms(1000);
    srand((uint32_t)time_us_32());

    while (1) {
        sequencia[fase - 1] = rand() % 3;
        mostrar_sequencia(sequencia, fase);

        uint8_t entrada[MAX_SEQ] = {0};
        ler_entrada(entrada, fase);

        bool erro = false;
        for (int i = 0; i < fase; i++) {
            if (entrada[i] != sequencia[i]) {
                erro = true;
                break;
            }
        }

        if (!erro) {
            fase++;
            sleep_ms(1000);
        } else {
            piscar_todos(3);
            fase = 1;
            sleep_ms(1000);
        }
    }
}
