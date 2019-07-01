#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include <string>
#include <sstream>   

void Rx_interrupt();
void writeStringToOLED(string str, int x, int y);
void showStatus();
void toggleLED();
void waterPlant();
void waterCritical();

void drawSmile();
void drawIdle();
void drawSleep();
void drawDead();

volatile bool sleepCycle;

/*
OLED
P5 -> MOSI
P6 -> aberto
P7 -> CLK
Pinos variáveis
P8 -> RES
P9 -> DC
P10 -> CS
Configuração SPI entra com (MOSI, MISO, CLK)
Configuração OLED entra com (SPI, DC, RES, CS, altura, largura)
*/
SPI spi(p5, p6, p7);
Adafruit_SSD1306_Spi spiOLED(spi, p9, p8, p10, 32, 128);

/*
BLUETOOTH
P13 -> TX
P14 -> RX
Configuração serial entra com (RX, TX)
*/


Serial hc(p14, p13);

/*
Componentes que utilizam leitura análogica:
P15 a P20
*/
AnalogIn ldr(p15);
AnalogIn hygroEarth(p16);
AnalogIn hygroWater(p17);

/*
Componentes que utilizam leitura digital:
Qualquer pino livre (P5 a P30)
*/
DigitalIn pres(p30);

volatile bool ledStatus;
volatile float waterLimit;
volatile time_t seconds;

int main()
{
    set_time(0);
    seconds = time(NULL);
    // Resetar display
    spiOLED.clearDisplay();
    drawSmile();
    spiOLED.display();
    wait(10);
    drawIdle();
    
    ledStatus = false;
    sleepCycle = false;
    
    int tsEarth = 0; //
    int tsLight = 0; //
    int tsPres = 0; //
    
    int delayEarth = 3600; //
    int delayLight; //
    int delayPres = 7200; //
    
    waterLimit = 1;
    float lightLimit = 1; //
    float earthLimit = 1; //
    
    bool turnOnLED;
    bool turnOffLED;
    
    while(1){
        seconds = time(NULL);
        
        /*
        HIGRÔMETRO TERRA
        - Rega a planta quando o nível de umidade detectado for adequado
        - Não rega se não tiver passado um determinado tempo desde a ultima vez
        
        Condição 1: Valor lido no higrômetro da terra maior (mais seco) que o limite
        Condição 2: Tempo desde a ultima vez que foi regado maior que o delay mínimo
        */
        if (hygroEarth.read() > earthLimit && seconds - tsEarth > delayEarth) {
            tsEarth = seconds;
            waterPlant();
        }
        
        /*
        SENSOR LDR E FITA DE LED
        - Liga/desliga a fita de LED dependendo do nível de luz
        (acrescentar: do horário do dia)
        
        Condição 1: Valor lido no LDR inferior a luz necessária
        Condição 2: LED está desligado
        Condição 3 (acrescentar): horário atual não ser noite
        */
        turnOnLED = ldr.read() < lightLimit && !ledStatus;
        // Mesmas condições, mas inversas para desligar
        turnOffLED = ldr.read() > lightLimit && ledStatus;
        if (turnOnLED || turnOffLED) {
            toggleLED();
        }
        
        /*
        SENSOR PRESENÇA
        - Desenha uma carinha feliz quando detecta presença de alguém ^u^
        
        Condição 1: Valor lido no sensor de presença é 1
        Condição 2: Tempo desde a ultima vez que foi ativado maior que o delay mínimo
        */
        if (pres.read() && seconds - tsPres > delayPres) {
            tsPres = seconds;
            drawSmile();
            wait(15);
        }
        
        /*
        HIGRÔMETRO ÁGUA
        - Interrompe o programa no caso de não ter água no tanque
        
        Condição 1: Valor lido no higrômetro da água maior (mais seco) que o limite 
        */
        if (hygroWater.read() > waterLimit) {
            waterCritical();
        }
        
        /*
        DORMIR
        - O vaso dorme quando o horário for de noite.
        
        Condição 1: ??? horas noturnas
        */
        
        // ... 
        if (true) {
            drawSleep();
        }
        
        wait(1);
    }
}

// Receber comandos manuais por bluetooth
void Rx_interrupt() {
    char c;
    c = hc.getc();
    switch(c){
        case 'S':
            showStatus();
            break;
        case 'A':
            waterPlant();
            break;
        case 'L':
            toggleLED();
            break;
        default:
            break;
    }
}

void writeStringToOLED(string str, int x, int y) {
    spiOLED.setTextCursor(x, y);
    spiOLED.fillRect(x, y, 128, 8, 0);
    for(int i = 0; i < str.length(); i++) {
        spiOLED.writeChar(str[i]);
    }
    spiOLED.display();
}

// Mostra todas as leituras atuais
void showStatus() {
    hc.printf("Presença: %d\nLDR: %.5f\nHigrometro Terra: %.5f\nHigrometro Agua: %.5f",
        pres.read(),
        ldr.read(),
        hygroEarth.read(),
        hygroWater.read());
}

// Rega a planta a força, desde que tenha água no reservatório
void waterPlant() {
    if(hygroWater.read() > waterLimit) waterCritical();
    // ...
}

// Interrompe o funcionamento do vaso até que seja alimentado com água
void waterCritical() {
    drawDead();
    
    while(hygroWater.read() > waterLimit) {
        // whatever?!
        
        wait(1);
    }
    
    drawIdle();
}

// Liga/desliga os LEDs
void toggleLED() {
    ledStatus = !ledStatus;
    // ...
}

// Desenha uma carinha feliz ^u^
void drawSmile() {
    spiOLED.drawCircle(12, 20, 9, 1);
    spiOLED.fillRect(0, 5, 22, 15, 0);
    spiOLED.fillTriangle(2, 15, 10, 15, 6, 7, 1);
    spiOLED.fillTriangle(3, 15, 9, 15, 6, 8, 0);
    spiOLED.fillTriangle(14, 15, 22, 15, 18, 7, 1);
    spiOLED.fillTriangle(15, 15, 21, 15, 18, 8, 0);
    spiOLED.setTextSize(2);
    writeStringToOLED("Hello!", 30, 10);
}

void drawIdle() {
    spiOLED.drawCircle(12, 20, 9, 1);
    spiOLED.fillRect(0, 5, 22, 15, 0);
    spiOLED.fillRect(7, 6, 2, 10, 1);
    spiOLED.fillRect(16, 6, 2, 10, 1);
    spiOLED.setTextSize(2);
    writeStringToOLED("All ok!", 40, 10);
}

void drawSleep() {
    int y = 1;
    while(sleepCycle) {
        spiOLED.clearDisplay();
        spiOLED.drawCircle(10, 23, 3, 1);
        spiOLED.fillRect(2, 12, 8, 2, 1);
        spiOLED.fillRect(15, 12, 8, 2, 1);
        spiOLED.setTextSize(1);
        writeStringToOLED("z", 35, 16 + y * 3);
        spiOLED.setTextSize(2);
        writeStringToOLED("z", 45, 11 - y * 3);
        spiOLED.setTextSize(3);
        writeStringToOLED("z", 65, 6 + y * 3);
        spiOLED.setTextSize(4);
        writeStringToOLED("z", 90, 1 - y * 3);
        wait(1);
        y = y * -1;
        
        // ...
        if (seconds > 86400) sleepCycle = false;
    }
}

void drawDead() {
    spiOLED.drawCircle(13, 27, 7, 1);
    spiOLED.fillRect(0, 28, 22, 15, 0);
    spiOLED.drawLine(2, 15, 10, 7, 1);
    spiOLED.drawLine(2, 7, 10, 15, 1);
    spiOLED.drawLine(16, 15, 24, 7, 1);
    spiOLED.drawLine(16, 7, 24, 15, 1);
    spiOLED.setTextSize(2);
    writeStringToOLED("Help!", 50, 6);
    spiOLED.setTextSize(1);
    writeStringToOLED("Need water!!", 50, 22);
}