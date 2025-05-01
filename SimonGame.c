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
#define NOTA_SI 247
#define NOTA_RE 293

// Notas musicais em Hz (oitava correta para a melodia)
#define DO3  131
#define RE3  147
#define MI3  165
#define FA3  175
#define SOL3 196
#define LA3  220
#define SI3  247
#define DO4  262  // Do' (agudo)
#define RE4  294
#define MI4  330
#define SOLs3 208 // Sol#
#define LAs3 233 // La#

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

void tocar_nota(int freq, int duracao_ms) {
    if (freq <= 0) {
        sleep_ms(duracao_ms);
        return;
    }

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A);
    uint chan = pwm_gpio_to_channel(BUZZER_A);
    
    // Configuração precisa do PWM
    float div = 125.0f; // Divisor de clock fixo para melhor precisão
    uint16_t wrap = (125000000 / div) / freq - 1;
    
    pwm_set_clkdiv(slice_num, div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap / 2);
    
    // Usa busy_wait para timing preciso
    busy_wait_us(duracao_ms * 1000);
    pwm_set_chan_level(slice_num, chan, 0); // Silencia
    busy_wait_us(50000); // Pequena pausa entre notas (50ms)
}


// Melodia Game Over melhorada
void tocar_derrota() {
    // Do' Sol Mi La Si La Sol# La# Sol# Mi Re Mi
    tocar_nota(DO4, 200);   // Do'
    tocar_nota(SOL3, 200);  // Sol
    tocar_nota(MI3, 200);   // Mi
    tocar_nota(LA3, 250);   // La
    tocar_nota(SI3, 280);   // Si
    tocar_nota(LA3, 300);   // La hjhj
    tocar_nota(SOLs3, 330); // Sol#
    tocar_nota(LAs3, 360);  // La#
    tocar_nota(SOLs3, 100); // Sol#
    tocar_nota(MI3, 50);   // Mi
    tocar_nota(RE3, 100);   // Re
    tocar_nota(MI3, 450);   // Mi (mais longo no final)
    
    // Pequena pausa no final
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
                tocar_nota(NOTA_RE, 300);
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
            tocar_nota(NOTA_RE, 200);
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

    // Inicializa GPIOs
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
        // Adiciona novo elemento à sequência
        sequencia[fase - 1] = rand() % 3;
        
        // Mostra a sequência ao jogador
        mostrar_sequencia(sequencia, fase);

        // Lê a entrada do jogador
        uint8_t entrada[MAX_SEQ] = {0};
        ler_entrada(entrada, fase);

        // Verifica se a entrada está correta
        bool erro = false;
        for (int i = 0; i < fase; i++) {
            if (entrada[i] != sequencia[i]) {
                erro = true;
                break;
            }
        }

        if (!erro) {
            // Acertou - avança para próxima fase
            fase++;
            sleep_ms(1000);
        } else {
            // Errou - game over
            piscar_todos(3);
            tocar_derrota(); // Toca a melodia do Super Mario
            fase = 1; // Reinicia o jogo
            sleep_ms(1000);
        }
    }
}
