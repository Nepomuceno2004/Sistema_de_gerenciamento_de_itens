#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "generated/ws2812.pio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// pinos dos botões
#define botaoA 5
#define botaoB 6

// pino do buzzer
#define BUZZER_PIN 21

// pinos dos led
#define ledAzul 12

// pinos da matriz de led
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// pinos para o display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define joystick_X 26      // GPIO para eixo X
#define joystick_Y 27      // GPIO para eixo Y

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

// função que recebe a frequência para emitir o som
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

// captura o centro do joystick
uint16_t get_center(uint8_t adc_channel)
{
    uint32_t sum = 0;
    for (int i = 0; i < 100; i++)
    {
        adc_select_input(adc_channel);
        sum += adc_read();
        sleep_ms(5);
    }
    return sum / 100;
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

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // pino SDA para a i2c
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // pino SCL para a i2c
    gpio_pull_up(I2C_SDA);                                        // Pull up SDA
    gpio_pull_up(I2C_SCL);                                        // Pull up SCL
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // inicialização do adc
    adc_init();
    adc_gpio_init(joystick_X);
    adc_gpio_init(joystick_Y);

    // configuração do centro do joystick
    uint16_t adc_value_x;
    uint16_t adc_value_y;
    uint16_t center_x = get_center(0);
    uint16_t center_y = get_center(1);

    // define as interrupções
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        // Leitura do Joystick
        adc_select_input(0); // Eixo X (pino 26)
        adc_value_x = adc_read();
        adc_select_input(1); // Eixo Y (pino 27)
        adc_value_y = adc_read();

        // Mapeamento do ADC para a tela, garantindo limites
        int y = (adc_value_y * 120) / 4095; // Mapeia o eixo X para 120
        y += 3;                             // Ajusta para começar a partir de 3

        if (y > 120)
            y = 120; // Limitação para evitar ultrapassar a borda

        int x = 56 - (adc_value_x * 56) / 4095; // Inverte o eixo X
        x += 3;

        if (x > 56)
            x = 56;
        if (x < 3)
            x = 3;

        ssd1306_fill(&ssd, false);                      // Limpa o display
        ssd1306_rect(&ssd, x, y, 8, 8, true, true);     // Desenha o quadrado na posição corrigida
        ssd1306_rect(&ssd, 2, 2, 124, 62, true, false); // Borda
        ssd1306_send_data(&ssd);                        // Atualiza o display

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
