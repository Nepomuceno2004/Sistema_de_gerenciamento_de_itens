#include <stdio.h>
#include "pico/stdlib.h"

#define botaoA 5
#define botaoB 6

int totalItens = 0;

volatile uint32_t last_time = 0;

void gpio_irq_handler(uint gpio, uint32_t event)
{

    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000)
    {
        if (gpio == botaoA)
        {
            totalItens++;
            printf("\nAVISO: Novo item adicionado. Quantidade total: %d\n", totalItens);
        }
        else
        {
            if (totalItens > 0)
            {
                totalItens--;
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

    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true)
    {
        sleep_ms(100);
    }
}
