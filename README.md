# Sistema de gerenciamento de itens
## ItemSys - Um sistema embarcado inteligente para gerenciamento eficiente de itens – compacto, preciso e sempre pronto.

O ItemSys é um sistema embarcado que auxilia no controle físico de itens em um pequeno depósito ou espaço de armazenamento. Utilizando um conjunto de sensores, botões, uma matriz de LEDs, buzzer e display OLED, o sistema permite adicionar ou remover itens com facilidade, fornecendo feedback visual e sonoro para cada ação. Ideal para makerspaces, laboratórios ou inventários pessoais.

## Funcionalidades
-  Contagem dinâmica de itens com feedback visual (símbolos de "+" e "-").
-  Alertas sonoros distintos para adição e remoção.
-  Interface com joystick para movimentação de cursor no display.
-  Display OLED com bordas e elementos gráficos atualizáveis.
-  Matriz WS2812B para indicar ações com animações coloridas.
-  Botões físicos para controle direto do inventário.

  ## Como Usar
  -  Certifique-se de ter as bibliotecas necessárias (como pico-sdk, ssd1306, ws2812.pio, etc.) corretamente incluídas no seu projeto CMake.
  -  Pressione Botão A para adicionar um item.
  -  Pressione Botão B para remover um item.
  -  O número total é atualizado e exibido via UART.
  -  Símbolos "+" ou "-" são mostrados na matriz de LEDs.
  -  O buzzer emite um som para confirmar a ação.
  -  O joystick movimenta um quadrado no display OLED, com limites definidos e bordas visuais.
  -  Durante o funcionamento, o sistema também exibe mensagens de status na UART.

 ## Componentes Utilizados
 -  Raspberry Pi Pico w
 -  Botões (x2)
 -  Buzzer
 -  Matriz de LEDs WS2812B
 -  Joystick Analógico
 -  Display OLED I2C (SSD1306)
 -  ADC
 -  LED RGB

   ## Autor
   ### Matheus Nepomuceno Souza
