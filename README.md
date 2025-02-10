Projeto LED WS2812 e Display SSD1306 com Raspberry Pi Pico

Este projeto utiliza o microcontrolador Raspberry Pi Pico e envolve a manipulação de LEDs WS2812, um display OLED SSD1306, e dois botões para acionar LEDs. O objetivo é exibir a palavra "FLAMENGO" e outros caracteres no display, além de controlar LEDs de diferentes cores. O código também permite interação via UART para receber caracteres e controlar os LEDs.

Componentes Utilizados
Raspberry Pi Pico: Controlador principal do projeto.
Display SSD1306 (I2C): Para exibir mensagens no formato de texto e gráficos simples.
LEDs WS2812: LEDs endereçáveis individualmente para mostrar caracteres e animações.
Botões (GPIO): Para alterar o estado de LEDs físicos no circuito.
LEDs RGB (Verde e Azul): LEDs físicos para acender ou apagar dependendo da interação.

Interrupções dos Botões
Quando o Botão A ou Botão B é pressionado, é acionada uma interrupção que altera o estado dos LEDs (verde ou azul).
main()
Função principal que realiza a inicialização de todos os periféricos (display, GPIO, UART), configura as interrupções dos botões e permite interação via UART. A função executa um loop onde verifica os estados dos botões e da UART, e aciona as funcionalidades apropriadas.

Compilação e Execução
Pré-requisitos
SDK do Raspberry Pi Pico instalado.
Toolchain de compilação (CMake, GCC) configurado corretamente.
Pico SDK configurado para compilar e carregar o código no Raspberry Pi Pico.

Após a compilação, carregue o arquivo .uf2 gerado no Raspberry Pi Pico.
Comunicação Serial
Conecte o Raspberry Pi Pico ao seu computador e abra uma porta serial para interagir com o dispositivo, por exemplo, com minicom ou PuTTY.

Conclusão
Esse projeto oferece uma maneira simples de controlar LEDs endereçáveis, um display OLED e realizar interações com o usuário usando botões e UART. É ideal para aprender a programar microcontroladores, controlar periféricos e trabalhar com interrupções e comunicação serial.
