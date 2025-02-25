#include <stdio.h> // Biblioteca padrão da linguagem C
#include "pico/stdlib.h" // Subconjunto central de bibliotecas do SDK Pico
#include "hardware/pio.h" // Biblioteca para controle do PIO utilizado para controle da Matriz de LEDs 5x5
#include "matriz.pio.h" // Arquivo .pio que contém a definição do programa
#include "hardware/i2c.h" // Biblioteca para controle da comunicação I2C
#include "include/ssd1306.h" // Biblioteca para controle do display OLED
#include "hardware/adc.h" // Biblioteca para controle da conversão ADC
#include "hardware/timer.h" // Biblioeca para controle do Timer
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

// Variáveis de controle do alerta
#define Alerta_ms 5000  // Tempo do alerta (ms)
#define Intervalo_us 15000000// 15s 180000000  // 3 minutos em microssegundos
volatile bool evento = false;  // Estado do evento para acionamento do buzzer
struct repeating_timer sirene_timer;  // Timer de hardware para acionamento do buzzer a cada Intervalo_us

// Variáveis Globais para permitir a função de saída da matriz de LEDs
PIO pio;
uint sm;
double intensity = 0.1;

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
        valor_led = matrix_rgb(intensity, intensity, intensity); // LED apagado para os espaços vazios
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

//Rotinas de Interrupção dos Botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time > 200000) { //Apenas ativa as funções quando o intervalo entre acionamentos é superior a 0.2 segundos
        last_time = current_time; //Atualiza o tempo do último evento
        if (gpio == Bot_Left) {
            // Ação do botão da esquerda
        } else if (gpio == Bot_Right) {
            // Ação do botão da direita
        } else if (gpio == Bot_Confirm) {
            // Ação do botão de Confirmação
        } else {
            // reset_usb_boot(0,0); //Habilita o modo de gravação do microcontrolador
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
    gpio_init(Matriz); // Inicializa o pino da matriz de LEDs
    gpio_set_dir(Matriz, GPIO_OUT); // Configura o pino da matriz de LEDs como saída
    gpio_init(Buzzer); // Inicializa o pino do buzzer
    gpio_set_dir(Buzzer, GPIO_OUT); // Configura o pino do buzzer como saída
    gpio_init(Relay); // Inicializa o pino do relé
    gpio_set_dir(Relay, GPIO_OUT); // Configura o pino do relé como saída
}

// Função de inicialização de protocolo de comunicação I2C, PWM e ADC
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
}

// Função para tocar o alerta (com verificação do evento)
void tocar_alerta() {
    if (!evento) return;  // Se o evento não estiver ativo, não toca
    
    printf("Tocando alerta...\n");
    // Função para tocar uma sirene alternando entre as notas Agudas e graves
    int tempo = 300;  // Tempo de cada nota (ms)
    int ciclos = Alerta_ms / (2 * tempo);  // Número de alternâncias

    for (int i = 0; i < ciclos; i++) {
        // Nota aguda
        int periodo1 = 1000000 / 400;
        int ciclos1 = (400 * tempo) / 1000;
        for (int j = 0; j < ciclos1; j++) {
            gpio_put(Buzzer, true);
            sleep_us(periodo1 / 2);
            gpio_put(Buzzer, false);
            sleep_us(periodo1 / 2);
        }
        printf("Nota aguda\n");
        sleep_ms(200);  // Pequena pausa entre notas

        // Nota grave
        int periodo2 = 1000000 / 250;
        int ciclos2 = (250 * tempo) / 1000;
        for (int j = 0; j < ciclos2; j++) {
            gpio_put(Buzzer, true);
            sleep_us(periodo2 / 2);
            gpio_put(Buzzer, false);
            sleep_us(periodo2 / 2);
        }
        printf("Nota grave\n");
        sleep_ms(200);  // Pequena pausa antes de repetir
    }
}

// Callback do Timer - dispara a cada 3 minutos
bool tocar_alerta_callback(struct repeating_timer *t) {
    tocar_alerta();
    return true; // Mantém a repetição do timer
}

int main() {
        
    stdio_init_all();

    init_pins();
    init_process();
    desenho();

    struct repeating_timer timer;
    
    // Inicia um timer que chama tocar_alerta_callback() após 3 minutos
    add_repeating_timer_us(Intervalo_us, tocar_alerta_callback, NULL, &timer);

    printf("Sistema iniciado! O buzzer tocará a cada 3 minutos se o evento estiver ativo.\n");

    while (true) {
        printf("Loop principal rodando...\n\n");
        sleep_ms(1000);
    }
}
