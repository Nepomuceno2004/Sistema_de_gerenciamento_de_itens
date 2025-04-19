#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "generated/ws2812.pio.h"

//pinos dos botões
#define botaoA 5
#define botaoB 6

//pinos da matriz de led
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

//total de itens no depósito
int totalItens = 0;

//variável que guarda o último pressionamento dos botões
volatile uint32_t last_time = 0;

//símbolo mais na matriz de led
bool simboloMais[NUM_PIXELS] = {
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0
};

//símbolo menos na matriz de led
bool simboloMenos[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};

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

//função de interrupção
void gpio_irq_handler(uint gpio, uint32_t event)
{

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000)
    {
        if (gpio == botaoA)
        {
            totalItens++;
            set_one_led(0, 2, 0, simboloMais);
            printf("\nAVISO: Novo item adicionado. Quantidade total: %d\n", totalItens);
        }
        else
        {
            if (totalItens > 0)
            {
                totalItens--;
                set_one_led(2, 0, 0, simboloMenos);
                printf("\nAVISO: Item Removido. Quantidade total: %d\n", totalItens);
            }
        }

        last_time = current_time;
    }
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

    // inicialização da matriz de led
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    //chamada das interrupções
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        sleep_ms(100);
    }
}
