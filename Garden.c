#include <stdio.h> // Biblioteca padrão da linguagem C
#include "pico/stdlib.h" // Subconjunto central de bibliotecas do SDK Pico
#include "hardware/pio.h" // Biblioteca para controle do PIO utilizado para controle da Matriz de LEDs 5x5
#include "matriz.pio.h" // Arquivo .pio que contém a definição do programa
#include "hardware/i2c.h" // Biblioteca para controle da comunicação I2C
#include "include/ssd1306.h" // Biblioteca para controle do display OLED
#include "hardware/adc.h" // Biblioteca para controle da conversão ADC
#include "hardware/pwm.h" // Biblioteca para controle do PWM
#include "pico/bootrom.h" // A ser retirado, permanece durante o periodo de desenvolvimento

// Definição dos pinos utilizados para entradas
#define Bot_Left 5
#define Bot_Right 6
#define Bot_Confirm 22
#define Sensor_Luz 26
#define Sensor_Umidade 27

// Definição dos pinos utilizados para entradas
#define Matriz 7
#define Buzzer 21
#define Relay 12

// Definições da comunicação I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define address 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display para todas as funções

// Definição de strings para exibição no display
char str_luz[7], str_umid[7], str_ideal[5], str_rega[4];

// Variáveis de controle com botões
int8_t ideal = 50 ; // Valor ideal de umidade do solo, inicializado em 50%
int8_t tempo_rega = 5; // Tempo de rega, inicializado em 5s

// Variáveis de controle do alerta
#define Alerta_ms 3000  // Tempo do alerta (ms)
volatile bool evento = false;  // Estado do evento para acionamento do buzzer
uint slice_num; // Número do slice do PWM

// Variáveis de controle da conversão ADC
uint16_t adc_value_luz, adc_value_umidade; // Valores lidos dos sensores de luminosidade e umidade do solo
double umidade = 50; // Variável para armazenar o valor de umidade do solo

// Variáveis Globais para permitir a função de saída da matriz de LEDs
PIO pio;
uint sm;
double intensidade = 0.5; // Intensidade inicial da matriz de LEDs, irá variar de acordo com a luminosidade do ambiente

// Variável para debounce dos botões
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

// Função de definição de dados para Matriz de LEDs
uint32_t matrix_rgb (double r, double g, double b) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função de acionamento da Matriz de LEDs
void desenho () {
    uint32_t valor_led;
    for (int16_t i = 0; i < 25; i++) {
        valor_led = matrix_rgb(intensidade, intensidade, intensidade-0.05);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

//Rotinas de Interrupção dos Botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000) { //Apenas ativa as funções quando o intervalo entre acionamentos é superior a 0.2 segundos
        last_time = current_time; //Atualiza o tempo do último evento
        if (gpio == Bot_Left) {
            if (ideal > 0) {
                ideal -= 10; // Aumenta o valor de umidade ideal
            } else {
                ideal = 0; // Limita o valor de umidade ideal em 0%
            }  
        } else if (gpio == Bot_Right) {
            if (ideal < 100) {
                ideal += 10; // Diminui o valor de umidade ideal
            } else {
                ideal = 100; // Limita o valor de umidade ideal em 100%
            }
        } else if (gpio == Bot_Confirm) {
            tempo_rega += 5; // Aumenta o tempo de rega
            if (tempo_rega > 40) {
                tempo_rega = 5; // Limita o tempo de rega em 40s
            }
            
            //reset_usb_boot(0,0); //Habilita o modo de gravação do microcontrolador (Ativado em Período de Testes)
        } else {
        }   
    }
}


// Função de inicialização de entradas, saídas e interrupções
void init_pins(){
    // Entradas e habilitação de interrupções
    gpio_init(Bot_Left); // Inicializa o pino do botão esquerdo
    gpio_set_dir(Bot_Left, GPIO_IN); // Configura o pino do botão esquerdo como entrada
    gpio_pull_up(Bot_Left); // Habilita o pull-up do botão esquerdo
    gpio_set_irq_enabled_with_callback(Bot_Left, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção por borda de descida no botão esquerdo
    gpio_init(Bot_Right); // Inicializa o pino do botão direito
    gpio_set_dir(Bot_Right, GPIO_IN); // Configura o pino do botão direito como entrada
    gpio_pull_up(Bot_Right); // Habilita o pull-up do botão direito
    gpio_set_irq_enabled_with_callback(Bot_Right, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção por borda de descida no botão direito
    gpio_init(Bot_Confirm); // Inicializa o pino do botão de confirmação
    gpio_set_dir(Bot_Confirm, GPIO_IN); // Configura o pino do botão de confirmação como entrada
    gpio_pull_up(Bot_Confirm); // Habilita o pull-up do botão de confirmação
    gpio_set_irq_enabled_with_callback(Bot_Confirm, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita a interrupção por borda de descida no botão de confirmação
    
    // Saídas
    gpio_init(Relay); // Inicializa o pino do relé
    gpio_set_dir(Relay, GPIO_OUT); // Configura o pino do relé como saída
    
    //Configurações da PIO e saída da Matriz de LEDs
    pio = pio0;
    uint offset = pio_add_program(pio, &matriz_program);
    sm = pio_claim_unused_sm(pio, true);
    matriz_program_init(pio, sm, offset, Matriz);
}

// Configura o PWM para uma frequência específica
void pwm_setup(uint32_t freq) {
    // Configura o pino para a função PWM
    gpio_set_function(Buzzer, GPIO_FUNC_PWM);

    // Obtém o slice de PWM associado ao pino
    slice_num = pwm_gpio_to_slice_num(Buzzer);

    // Obtém o clock do sistema
    uint32_t clock = clock_get_hz(clk_sys);

    // Calcula o divisor e wrap para obter a frequência desejada
    uint32_t divider16 = (clock << 4) / freq;
    uint32_t wrap = (divider16 + (1 << 4) - 1) >> 4;
    
    // Configura o PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); // Ajusta o divisor de clock conforme necessário
    pwm_init(slice_num, &config, true);

    // Define o contador de wrap e o duty cycle para 50%
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(Buzzer), wrap / 2);
    pwm_set_enabled(slice_num, true); // Ativa o PWM antes do loop
}

// Função de callback para desligar o Relé após o tempo programado.
int64_t turn_off_callback(alarm_id_t id, void *user_data) {
    
    if (!evento){
        return 0;
    } else {
        // Desliga o Relé configurando o pino Relay para nível baixo.
        gpio_put(Relay, false);
        // Retorna 0 para indicar que o alarme não deve se repetir.
        return 0;
    }
}

// Função para tocar o alerta alternando entre 250 Hz e 400 Hz
void tocar_alerta() {
    printf("Verificando alerta...\n");
    sleep_ms(4000); // Aguarda 4 segundos antes de tocar o alerta, verifica se a umidade ainda está abaixo do ideal mesmo com a bomba ligada
    evento = (gpio_get(Relay) && umidade*100 < (ideal - 20)); // Verifica se o evento de alerta precisa ser acionado
    if (!evento) {
        printf("Alerta cancelado...\n");
        return;
    } else {
        printf("Tocando alerta...\n");
        pwm_set_enabled(slice_num, true); // Ativa o PWM antes do loop
        int tempo_nota = 250;  // Tempo de cada nota (ms)
        int ciclos = Alerta_ms / (2 * tempo_nota);  // Número de alternâncias
        add_alarm_in_ms(Alerta_ms, turn_off_callback, NULL, false); // Agenda o alarme para tocar o alerta novamente após 500ms
        for (int i = 0; i < ciclos; i++) {
            pwm_setup(250);
            sleep_ms(tempo_nota);
            pwm_setup(400);
            sleep_ms(tempo_nota);
        }
        pwm_set_enabled(slice_num, false); // Desliga o PWM
        printf("Alerta desligado...\n");
    }
}

// Callback que verifica evento e toca o alerta após 1 segundo
int64_t verificar_alerta_callback(alarm_id_t id, void *user_data) {
    if (evento) {
        tocar_alerta(); // Se o evento ainda for verdadeiro, toca o alerta
    }
    return 0; // Alarme não se repete
}

// Função de inicialização de protocolo de comunicação I2C e ADC
void init_process(){
    // Inicialização da comunicação I2C. Utilizando a frequência de 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino para I2C
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, address, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    
    adc_gpio_init(Sensor_Luz); // Inicializa o pino do sensor LDR
    adc_gpio_init(Sensor_Umidade); // Inicializa o pino do sensor de umidade
    adc_init(); // Inicializa o ADC
}    

int main() {
        
    stdio_init_all();

    init_pins();
    init_process();
    //tocar_alerta();
    //desenho();

    // Váriáveis utilizadas na main()
    pio = pio0;

    while (true) {
        
        ssd1306_fill(&ssd, 0); // Limpa o display
        
        evento = (gpio_get(Relay) && umidade*100 < (ideal - 20)); // Verifica se o evento de alerta precisa ser acionado

        if (evento) {
            tocar_alerta(); // Toca o alerta
            evento = false; // Reseta o evento
        } else {
            // Acionamento da Iluminação de acordo com a luminosidade do ambiente
            adc_select_input(0); // Seleciona o canal 0 do ADC, que corresponde ao pino 26, contendo o sensor de luminosidade
            adc_value_luz = adc_read(); // Realiza a leitura do valor de luminosidade
            intensidade = 1 - adc_value_luz/4095.0; // Ajusta a intensidade da matriz de LEDs de acordo com o valor de luminosidade
            printf("Luminosidade: %d (%.0f%%)\n", adc_value_luz, (intensidade*100)); // Exibe o valor de luminosidade
            sprintf(str_luz, "%.0f%%", (1-intensidade)*100);  // Converte o inteiro em string
            ssd1306_draw_string(&ssd, "Luminosidade:", 3, 4); // Escreve a intensidade de luz no display
            ssd1306_draw_string(&ssd, str_luz, 102, 4); // Escreve a porcenagem de luz no display
            desenho(); // Atualiza a matriz de LEDs de acordo com a intensidade de luz
    
            
            // Definições de parâmentros de rega
            ssd1306_draw_string(&ssd, "Umidade Ideal:", 3, 28); // Escreve o nível de umidade ideal para a planta no display
            sprintf(str_ideal, "%d%%", ideal);  // Converte o inteiro em string
            ssd1306_draw_string(&ssd, str_ideal, 102, 28); // Escreve a porcenagem de umidade ideal no display
            ssd1306_draw_string(&ssd, "Tempo de Rega:", 3, 40); // Escreve o tamanho do vaso no display
            sprintf(str_rega, "%ds", tempo_rega);  // Converte o inteiro em string
            ssd1306_draw_string(&ssd, str_rega, 102, 40); // Escreve o tempo de rega no display
            
            // Acionamento do Relé de acordo com a umidade do solo
            adc_select_input(1); // Seleciona o canal 1 do ADC, que corresponde ao pino 27, contendo o sensor de umidade do solo
            adc_value_umidade = adc_read(); // Realiza a leitura do valor de umidade do solo
            umidade = adc_value_umidade/4095.0; // Ajusta a intensidade da matriz de LEDs de acordo com o valor de luminosidade
            printf("Umidade do solo: %d (%.0f%%)\n", adc_value_umidade, (umidade*100)); // Exibe o valor de umidade do solo
            sprintf(str_umid, "%.0f%%", umidade*100);  // Converte o valor percentual em string
            ssd1306_draw_string(&ssd, "Umidade do solo:", 3, 16); // Escreve a umidade do solo no display
            ssd1306_draw_string(&ssd, str_umid, 102, 16); // Escreve a porcenagem de umidade no display
            if (umidade*100 < (ideal - 20)) {
                gpio_put(Relay, 1); // Liga o relé
                ssd1306_draw_string(&ssd, "Irrigando...", 3, 52); // Escreve a mensagem de "irrigação" no display
                // Agendar o desligamento do relé após o tempo de rega
                add_alarm_in_ms((tempo_rega * 1000), turn_off_callback, NULL, false);

                // Agendar a verificação do alerta 1 segundo depois
                //add_alarm_in_ms(1000, verificar_alerta_callback, NULL, false);
            } else if (umidade*100 > (ideal - 20) && umidade*100 < (ideal + 20)) {
                ssd1306_draw_string(&ssd, "Rega em breve!", 3, 52); // Escreve a mensagem de "rega em breve" no display
            } else if (umidade*100 > (ideal + 20)) {
                ssd1306_draw_string(&ssd, "Solo umido!", 3, 52); // Escreve a mensagem de "solo umido" no display
            }
            
        }
        
        ssd1306_rect(&ssd, 1, 1, 126, 62, 1, 0); // Borda do display
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(100); // Aguarda 100ms para não sobrecarregar a CPU

    }
}
