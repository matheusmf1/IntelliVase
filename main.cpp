#include "mbed.h"
#include "Adafruit_SSD1306.h"
#include "ds1307.h"
#include <string>
#include <sstream>
#include <map>

#define DRY_EARTH_LIMIT 0.75
#define REGULAR_EARTH_LIMIT 0.50
#define MOIST_EARTH_LIMIT 0.25
#define LUMINOSITY_LIMIT 0.75
#define WATER_LIMIT 0.99

using namespace std;

/*** FUNCTIONS DECLARATIONS ***/
void BT_showMenu();
void BT_showStatus();
void BT_showHumidityTypes();

void LED_toggle();
void PUMP_water();
void TANK_empty();
bool EARTH_setHumidity(char, float&);
void STATE_toggleSleep();

void OLED_drawSmile();
void OLED_drawIdle();
void OLED_drawSleep(int);
void OLED_drawDead();
void OLED_writeString(string, int, int);

void DEBUG_run();

/*** GLOBAL VARIABLES ***/
SPI spi(p5, p6, p7);
Adafruit_SSD1306_Spi oled(spi, p24, p23, p25, 32, 128);

DS1307 rtc(p9, p10);

AnalogIn ldr(p15);
AnalogIn hygroEarth(p16);
AnalogIn hygroWater(p17);

DigitalIn presence(p18);

PwmOut pump(p21);
PwmOut led(p22);

Serial bt(p28, p27);

bool STATE_sleep = false;

int main() {
  /*** DEBUG FLOW ***/
  //DEBUG_run(); // Uncomment this line only when debugging

  /*** MAIN FLOW ***/
  /*** INITIALIZATION ***/
  set_time(0);
  bt.baud(9600);
  // Attach the BT_interrupt as the function to be called when receiving information through this serial
  //bt.attach(&BT_interrupt, Serial::RxIrq);

  time_t TS_internal;
  long TS_earth = -125;
  long TS_light = -65;
  long TS_presence = -65;
  long TS_notifyBT = -65;

  int TIME_second = 0;
  int TIME_minute = 0;
  int TIME_hour = 0;
  int TIME_day = 0;
  int TIME_date = 0;
  int TIME_month = 0;
  int TIME_year = 0;

  int DELAY_earth = 120;
  int DELAY_light = 60;
  int DELAY_presence = 60;
  int DELAY_notifyBT = 60;

  float EARTH_limit = REGULAR_EARTH_LIMIT;

  map<char, void(*)()> USER_options;
  USER_options['I'] = &BT_showStatus;
  USER_options['R'] = &PUMP_water;
  USER_options['L'] = &LED_toggle;
  USER_options['U'] = &BT_showHumidityTypes;
  USER_options['S'] = &STATE_toggleSleep;

  char USER_lastOption;
  char USER_actualOption;
  bool USER_keepLightOn = false;
  bool USER_keepSleepStateOn = false;
  
  bool LED_toggleOn;
  bool LED_toggleOff;

  int animationSwitch = 1;

  map<char, void(*)()>::iterator it;

  oled.clearDisplay();
  OLED_drawSmile();
  oled.display();
  wait(10);
  OLED_drawIdle();

  while (true) {
    /*** UPDATE VARIABLES ***/
    TS_internal = time(NULL);
    rtc.gettime(&TIME_second, &TIME_minute, &TIME_hour, &TIME_day, &TIME_date, &TIME_month, &TIME_year);

    STATE_sleep = (USER_keepSleepStateOn || (TIME_hour >= 20 && TIME_hour < 5)) ? true : false;

    
    LED_toggleOn = ldr.read() > LUMINOSITY_LIMIT && !led.read();
    LED_toggleOff = ldr.read() < LUMINOSITY_LIMIT && led.read();
  
    animationSwitch *= -1;

    /*** CHECK AND RESPOND BLUETOOTH INPUTS ***/
    if (TS_internal - TS_notifyBT > DELAY_notifyBT) {
      BT_showMenu();
      TS_notifyBT = TS_internal;
    }

    if (bt.readable()) {
      USER_lastOption = USER_actualOption;
      USER_actualOption = bt.getc();

      if (USER_lastOption == 'U') {
        bool succeeded = EARTH_setHumidity(USER_actualOption, EARTH_limit);

        if (succeeded) {
          bt.printf("\nAlteração realizada com sucesso!\n");
        } else {
          bt.printf("\nOpção inválida!\n");
        }

        continue;
      }

      //USER_options[op]();
      it = USER_options.find(USER_actualOption);

      if (it == USER_options.end()) {
        bt.printf("\nOpção inválida!\n");
        continue;
      }

      it->second();

      if (USER_actualOption == 'L') {
        USER_keepLightOn = (led.read() == 0) ? false : true;
      } else if (USER_actualOption == 'S') {
        USER_keepSleepStateOn = !USER_keepSleepStateOn;
      }
    }

    /*** SLEEP STATE ***/
    if (STATE_sleep) {
      if (!USER_keepLightOn && led.read() == 1) {
        LED_toggle();
      }

      OLED_drawSleep(animationSwitch);
    }
    /*** REGULAR STATE ***/
    else {
      /*** CHECK THE SENSORS AND ACT ACCORDINGLY ***/
      if (TS_internal - TS_presence > DELAY_presence && presence.read()) {
        OLED_drawSmile();
        wait(10);
        OLED_drawIdle();
        TS_presence = TS_internal;
      }
      
      if (!USER_keepLightOn && TS_internal - TS_light > DELAY_light && (LED_toggleOn || LED_toggleOff)) {
        LED_toggle();
        TS_light = TS_internal;
      }

      if (TS_internal - TS_earth > DELAY_earth && hygroEarth.read() > EARTH_limit) {
        OLED_drawIdle();
        PUMP_water();
        TS_earth = TS_internal;
      }
    }

    wait(0.1);
  }
}

/*** FUNCTIONS DEFINITIONS ***/
// Mostra o menu principal
void BT_showMenu() {
  bt.printf("\nOpções:\nI - Informações dos sensores\nR - Regar a plantar\nL - Ligar/Desligar leds\nU - Umidade da terra a ser mantida\nS - Ligar/Desligar modo sleep\n");
}

// Mostra todas as leituras atuais dos sensores
void BT_showStatus() {
  bt.printf(
    "\nLDR: %.5f\nHigrômetro Terra: %.5f\nHigrômetro Água: %.5f\n",
    ldr.read(),
    hygroEarth.read(),
    hygroWater.read()
  );
}

// Mostra os tipos de umidades de terra para a planta
void BT_showHumidityTypes() {
  bt.printf("\nOpções de umidade da terra:\nB - Baixa\nM - Média\nA - Alta\n");
}

// Liga/Desliga a fita de LED
void LED_toggle() {
  int toggle = (led.read() == 0) ? 1 : 0;
  led.write(toggle);
}

// Bombeia água para regar a planta se o reservatório não estiver vazio
void PUMP_water() {
  if (hygroWater.read() > WATER_LIMIT) {
    TANK_empty();
    return;
  }

  pump.write(1);
  wait(2); // tempo de irrigação
  pump.write(0);
}

// Informa que o reservatório de água precisa ser reabastecido
void TANK_empty() {
  OLED_drawDead();
  bt.printf("\n***ATENÇÃO***\nNÍVEL DE ÁGUA MUITO BAIXO!!!\nFAVOR REABASTECER O RESERVATÓRIO.\n");
}

// Configura o nível de umidade de terra da planta
bool EARTH_setHumidity(char type, float& limit) {
  if (type == 'B') {
    limit = DRY_EARTH_LIMIT;
  } else if (type == 'M') {
    limit = REGULAR_EARTH_LIMIT;
  } else if (type == 'A') {
    limit = MOIST_EARTH_LIMIT;
  } else {
    return false;
  }

  return true;
}

void STATE_toggleSleep() {
    STATE_sleep = !STATE_sleep;
}

// Mostra no display uma mensagem de receptividade
void OLED_drawSmile() {
  oled.clearDisplay();
  oled.drawCircle(12, 20, 9, 1);
  oled.fillRect(0, 5, 22, 15, 0);
  oled.fillTriangle(2, 15, 10, 15, 6, 7, 1);
  oled.fillTriangle(3, 15, 9, 15, 6, 8, 0);
  oled.fillTriangle(14, 15, 22, 15, 18, 7, 1);
  oled.fillTriangle(15, 15, 21, 15, 18, 8, 0);
  oled.setTextSize(2);
  OLED_writeString("Hello!", 40, 10);
}

// Mostra no display uma mensagem de normalidade
void OLED_drawIdle() {
  oled.clearDisplay();
  oled.drawCircle(12, 20, 9, 1);
  oled.fillRect(0, 5, 22, 15, 0);
  oled.fillRect(7, 6, 2, 10, 1);
  oled.fillRect(16, 6, 2, 10, 1);
  oled.setTextSize(2);
  OLED_writeString("All ok!", 40, 10);
}

// Mostra no display uma mensagem de sono
void OLED_drawSleep(int animationSwitch) {
  oled.clearDisplay();
  oled.drawCircle(10, 23, 3, 1);
  oled.fillRect(2, 12, 8, 2, 1);
  oled.fillRect(15, 12, 8, 2, 1);
  oled.setTextSize(1);
  OLED_writeString("z", 35, 16 + animationSwitch * 3);
  oled.setTextSize(2);
  OLED_writeString("z", 45, 11 - animationSwitch * 3);
  oled.setTextSize(3);
  OLED_writeString("z", 65, 6 + animationSwitch * 3);
  oled.setTextSize(4);
  OLED_writeString("z", 90, 1 - animationSwitch * 3);
  wait(1);
}

// Mostra no display uma mensagem de caos
void OLED_drawDead() {
  oled.clearDisplay();
  oled.drawCircle(13, 27, 7, 1);
  oled.fillRect(0, 28, 22, 15, 0);
  oled.drawLine(2, 15, 10, 7, 1);
  oled.drawLine(2, 7, 10, 15, 1);
  oled.drawLine(16, 15, 24, 7, 1);
  oled.drawLine(16, 7, 24, 15, 1);
  oled.setTextSize(2);
  OLED_writeString("Heelp!", 40, 6);
  oled.setTextSize(1);
  OLED_writeString("Need water!!", 40, 22);
}

// Escreve o texto passado como parâmetro no display
void OLED_writeString(string str, int x, int y) {
  oled.setTextCursor(x, y);
  oled.fillRect(x, y, 128, 8, 0);

  for (int i = 0; i < str.length(); i++) {
    oled.writeChar(str[i]);
  }

  oled.display();
}

// Inicia e trava a execução do programa para auxiliar o debugging do mesmo
void DEBUG_run() {
  set_time(0);

  time_t TS_internal;

  int second = 0;
  int minute = 0;
  int hour = 0;
  int day = 0;
  int date = 0;
  int month = 0;
  int year = 0;
  
  bool togglePWMs = false;
  int state;

  oled.clearDisplay();
  OLED_writeString("*** DEBUG MODE ON ***", 0, 14);

  wait(10);
  int rtcReturn = rtc.settime(40, 29, 59, 3, 25, 6, 19);
  bt.printf("%d", rtcReturn);
  wait(10);

  rtcReturn = rtc.gettime(&second, &minute, &hour, &day, &date, &month, &year);
  bt.printf("%d", rtcReturn);
  wait(10);

  while (true) {
    TS_internal = time(NULL);
    rtc.gettime(&second, &minute, &hour, &day, &date, &month, &year);

    bt.printf(
      "\n*** DEBUG MODE ON ***\nRTC: %.2D/%.2D/%.2D (%.2D) %.2D:%.2D:%.2D\nTimestamp Interno: %d\nLDR: %.5f\nHigrômetro Terra: %.5f\nHigrômetro Água: %.5f\nPresença: %d\nBomba: %.1f\nLED: %.1f\n",
      date, month, year, day, hour, minute, second,
      TS_internal,
      ldr.read(),
      hygroEarth.read(),
      hygroWater.read(),
      presence.read(),
      pump.read(),
      led.read()
    );

    wait(1);

    togglePWMs = !togglePWMs;
    state = togglePWMs ? 1 : 0;

    led.write(state);
    //pump.write(state);
  }
}
