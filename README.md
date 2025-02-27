# 🌱 Sistema Automatizado de Irrigação e Iluminação para Plantas

## 📌 Descrição do Projeto
Este projeto propõe um sistema automatizado de **irrigacão e iluminação para plantas**, utilizando um **Raspberry Pi Pico W (RP2040)** como unidade central de processamento. O sistema monitora as condições ambientais e realiza ajustes automáticos para garantir um ambiente adequado para o desenvolvimento das plantas.

### 🛠️ Funcionalidades Principais
- **Monitoramento e Acionamento de Iluminação**: Um sensor LDR capta a intensidade luminosa do ambiente. O valor lido é convertido pelo ADC do RP2040 e processado para controle de uma **matriz de LEDs RGB (WS2812 5x5)**, garantindo iluminação artificial ajustável.
- **Monitoramento da Umidade do Solo**: Um **sensor de umidade do solo** mede a umidade do substrato. Caso o nível esteja abaixo do configurado, um **relé aciona a bomba de água** para irrigar a planta. O sistema interrompe a irrigação após um intervalo de tempo ajustável.
- **Configuração de Parâmetros de Irrigação**: Push-buttons permitem que o usuário ajuste os **níveis de umidade desejados** e a **duração de cada rega**. As configurações são exibidas no **display OLED SSD1306**.
- **Alertas Sonoros**: Um **buzzer passivo** emite alertas para **baixo nível de água no reservatório** ou falhas nos sensores.
- **Interface com o Usuário via Display OLED**: O **display OLED SSD1306** exibe **dados em tempo real** da iluminação ambiente, umidade do solo e ajustes definidos pelo usuário.

---

## 🎯 Justificativa
O projeto propõe a otimização do **uso da água e da energia**, garantindo um ambiente adequado para o crescimento das plantas. Os principais benefícios incluem:
- **Eficiência hídrica**: O sensor de umidade do solo evita desperdícios, acionando a irrigação apenas quando necessário.
- **Regulação automática da iluminação**: A matriz de LEDs RGB ajusta a intensidade luminosa conforme as condições do ambiente.
- **Monitoramento fácil e interativo**: A interface do display OLED permite um acompanhamento em tempo real.
- **Sustentabilidade**: O sistema reduz a necessidade de monitoramento manual e incentiva práticas de cultivo mais ecológicas.

---

## 🚀 Originalidade e Diferenciais
O projeto se destaca ao integrar **diferentes tecnologias** para otimização do cultivo, com os seguintes diferenciais:
- **Automatização completa**: Integra sensores de **umidade do solo e iluminação**, proporcionando uma resposta dinâmica às necessidades da planta.
- **Controle de iluminação adaptativo**: A matriz de **LEDs RGB WS2812B 5x5** ajusta automaticamente a iluminação para otimizar o crescimento das plantas.
- **Eficiência no uso de recursos**: O controle da **bomba de água (RS385 12V) via relé** minimiza o consumo excessivo de água.
- **Baixo consumo energético**: O uso do **Raspberry Pi Pico W** permite maior eficiência energética e flexibilidade na programação.
- **Interface intuitiva**: O **display OLED SSD1306** facilita o monitoramento e ajuste dos parâmetros via push-buttons.

Este sistema **reduz a dependência do monitoramento manual**, promovendo um cultivo mais eficiente e sustentável, sendo aplicável tanto para **cultivos domésticos quanto para estufas agrícolas**.

---

## 🛠️ Componentes Utilizados

| Componente                        | Função |
|-----------------------------------|--------------------------------------------------------------------|
| **Raspberry Pi Pico W (RP2040)**  | Microcontrolador central |
| **Sensor LDR**                    | Mede a intensidade luminosa do ambiente |
| **Sensor de Umidade do Solo**      | Mede a umidade do solo para controle de irrigação |
| **Push-buttons**                   | Permitem ajuste dos níveis de irrigação |
| **Display OLED 128x64 (SSD1306)**  | Exibe informações sobre umidade e iluminação |
| **Buzzer Passivo (MLT8530)**       | Gera alertas sonoros para baixo nível de água |
| **Matriz de LEDs RGB (WS2812 5x5)** | Iluminação artificial ajustável |
| **Módulo Relé de 1 Canal (3V)**  | Aciona a bomba de água conforme necessidade |
| **Bomba de Água 12V (RS385)**    | Realiza a irrigação automatizada |

---

## 📌 Como Rodar o Projeto

### **1. Softwares Necessários**
- **VS Code** com extensão **Raspberry Pi Pico** instalada.
- **CMake** e **Ninja** configurados.
- **SDK do Raspberry Pi Pico** corretamente instalado.
- **Simulador Wokwi** integrado ao VS Code.

### **2. Clonando o Repositório**
```bash
    git clone https://github.com/EdnaRodrigues/Garden.git
    cd Garden
```

### **3. Compilando e Executando o Projeto**
1. **Importar o Projeto** no VS Code via extensão do Raspberry Pi Pico.
2. **Compilar o Código** usando a opção **Compile Project**.
3. **Carregar o Arquivo `.uf2`** na placa Raspberry Pi Pico W.
4. **Monitorar Execução** via Serial Monitor do VS Code.

### **4. Simulação no Wokwi**
1. Abra o arquivo `diagram.json` incluso no repositório.
2. Configure corretamente os componentes.
3. Execute a simulação no Wokwi clicando em **Play**.

---

## 📽️ Demonstração
[Adicionar link para vídeo ou imagens do projeto em funcionamento](https://youtu.be/s0fULUZS2p4)

---

## ✉️ Contato
Caso tenha dúvidas ou sugestões, entre em contato pelo e-mail: **ednards2002@gmail.com**.
