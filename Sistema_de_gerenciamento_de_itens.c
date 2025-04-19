#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "generated/ws2812.pio.h"
#include "hardware/pwm.h"

// pinos dos botões
#define botaoA 5
#define botaoB 6

// pino do buzzer de adicionar
#define BUZZER_PIN 21

// pinos dos led
#define ledAzul 12

// pinos da matriz de led
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// flag do buzzer adicionar
volatile bool som_adicionar = false;
volatile bool som_remover = false;

// total de itens no depósito
volatile int totalItens = 0;

// variável que guarda o último pressionamento dos botões
volatile uint32_t last_time = 0;

// símbolo mais na matriz de led
bool simboloMais[NUM_PIXELS] = {
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0};

// símbolo menos na matriz de led
bool simboloMenos[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0};

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Converte valores RGB para o formato de 32 bits utilizado pelos LEDs WS2812.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Atualiza os LEDs da matriz de acordo com o número a ser exibido.
void set_one_led(uint8_t r, uint8_t g, uint8_t b, bool led_buffer[])
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// função de interrupção
void gpio_irq_handler(uint gpio, uint32_t event)
{

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000)
    {
        if (gpio == botaoA)
        {
            totalItens++;                                                                // incrementa o item
            set_one_led(0, 2, 0, simboloMais);                                           // manda o símbolo para a matriz de led
            som_adicionar = true;                                                        // aciona flag
            printf("\nAVISO: Novo item adicionado. Quantidade total: %d\n", totalItens); // mensagem uart
        }
        else
        {
            if (totalItens > 0)
            {
                totalItens--;                                                         // desconta o item
                set_one_led(2, 0, 0, simboloMenos);                                   // manda o símbolo para a matriz de led
                som_remover = true;                                                   // aciona flag
                printf("\nAVISO: Item Removido. Quantidade total: %d\n", totalItens); // mensagem uart
            }
        }

        last_time = current_time; // atualiza a variável último tempo de acionamento
    }
}

void somBuzzer(uint freq, uint duration_ms)
{
    // Define o pino do buzzer como saída PWM
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    // Obtém o número do "slice" PWM associado ao pino
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Calcula o valor do contador 'top' com base na frequência desejada - 125 MHz/freq
    uint top = 125000000 / freq;

    // Define o valor máximo do contador PWM (wrap)
    pwm_set_wrap(slice, top);

    // Define o duty cycle do canal PWM
    // Aqui está usando 80% do valor máximo para um volume mais alto
    pwm_set_chan_level(slice, pwm_gpio_to_channel(BUZZER_PIN), (top * 8) / 10);

    // Ativa a saída PWM para gerar o som
    pwm_set_enabled(slice, true);
    gpio_put(ledAzul, true);

    // Mantém o som ativo pelo tempo passado pelo parâmetro
    sleep_ms(duration_ms);

    // Desliga o som
    pwm_set_enabled(slice, false);
    gpio_put(ledAzul, false);

    // Pequena pausa entre tons
    sleep_ms(20);
}

int main()
{
    stdio_init_all();

    // inicialização do botão A
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);

    // inicialização do botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);

    // inicialização do led
    gpio_init(ledAzul);
    gpio_set_dir(ledAzul, GPIO_OUT);

    // inicialização da matriz de led
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // define as interrupções
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        if (som_adicionar)
        {
            // envia a frequência do som
            somBuzzer(1200, 100);
            somBuzzer(1600, 100);
            som_adicionar = false; // desliga flag
        }
        if (som_remover)
        {
            // envia a frequência do som
            somBuzzer(1000, 120);
            somBuzzer(600, 150);
            som_remover = false; // desliga flag
        }
        sleep_ms(100);
    }
}
