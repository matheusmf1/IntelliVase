# Projeto: IntelliVase
O projeto visa criar um vaso inteligente que possa automaticamente cuidar de uma planta, provendo água e iluminação para a planta quando necessário, além de apresentar o estado atual da planta a partir de um display LCD e de uma comunicação Bluetooth.

## Componentes utilizados
- Sensor de umidade de solo (Higrômetro)
- Sensor de luz (LDR)
- Sensor de presença (HC-SR501)
- Bluetooth (HC08)
- Real Time Clock (DS1307)
- Display OLED (Adafruit SSD1306 128x32)
- Mini bomba de água
- Fita de LEDs

## Integrantes
Nome | RA | GitHub
------------ | ------------- | -------------
Felipe Andrade | 15.00175-0 | Kaisen-san
Matheus Mandotti | 16.00177-0 | matheusmf1
Vinícius Pereira | 16.03343-4 | VinPer

## Disposição dos componentes no Mbed
*Pino Mbed -> Pino device

  ### OLED
    Pinos fixos:
      P5 -> MOSI
      P6 -> MISO (display utilizado não possui pino MISO)
      P7 -> CLK

    Pinos variáveis (P21 à P26):
      P23 -> RES
      P24 -> DC
      P25 -> CS

    Declaração SPI: SPI(MOSI, MISO, CLK)
    Declaração OLED: Adafruit_SSD1306_Spi(SPI, DC, RES, CS, altura, largura)

  ### BLUETOOTH
    Pinos fixos:
      P27 -> TX
      P28 -> RX

    Declaração BLUETOOTH: Serial(RX, TX)

  ### BOMBA DE ÁGUA
    Pinos variáveis (P21 à P26):
      P21 -> IN3 (da ponte H)

    Fios:
      Marrom -> VCC (12V)
      Azul -> GND

    Declaração BOMBA: PwmOut(IN3)

  ### FITA DE LED
    Pinos variáveis (P21 à P26):
      P22 -> IN2 (da ponte H);

    Fios:
      Vermelho -> VCC (12V)
      Preto -> GND

    Declaração LED: PwmOut(IN2)

  ### LDR
    Pinos variáveis (P15 à P20):
      P15 -> AO

    Declaração LDR: AnalogIn(A0)

  ### HIGRÔMETRO - Terra
    Pinos variáveis (P15 à P20):
      P16 -> AO

    Declaração HIGROMETRO: AnalogIn(A0)

  ### HIGRÔMETRO - Água
    Pinos variáveis (P15 à P20):
      P17 -> AO

    Declaração HIGROMETRO: AnalogIn(A0)

  ### SENSOR DE PRESENÇA
    Pinos variáveis (P5 à P30):
      P18 -> OUT

    Declaração HIGROMETRO: AnalogIn(OUT)

  ### RTC
    Pinos fixos:
      P9 -> SDA
      P10 -> SCL

    Declaração RTC: DS1307(SDA, SCL)



## Funcionamento do programa

  ### HIGRÔMETRO (ÁGUA) E BOMBA DE ÁGUA
    - Informa o usuário para reabastecer o reservatório de água, caso o nível de água esteja muito baixo
    
    Lembrando que quanto menor o nível, maior o valor do sensor.

  ### HIGRÔMETRO (TERRA)
    - Rega a planta quando o nível de umidade detectado estiver abaixo do configurado
    - Não rega a planta se não tiver decorrido um determinado tempo desde a última irrigação
    
    Lembrando que quanto menor a umidade, maior o valor do sensor.

  ### LDR, FITA DE LED E RTC
    - Mantem a fita de LED sempre ligada caso o usuário tenha configurado assim
    - Caso contrário, liga/desliga a fita de LED se o nível de luz durante o dia estiver muito baixo
    - Mantem a luz ligada/desligada durante um determinado tempo desde a última mudança
    
    Lembrando que quanto menor a luminosidade, maior o valor do sensor.

  ### SENSOR DE PRESENÇA
    - Reage por meio do display quando detecta presença de alguém
    - Não faz nada se não tiver decorrido um determinado tempo desde a última presença detectada



## Fluxos do programa

  ### Principal
  ```
  --> Configuração inicial
  --> while (true)
  ----> Atualiza as variáveis
  ----> Checa e responde aos comandos enviados por Bluetooth
  ----> if isSleepTime
  ------> // Entra no estado de sleep
  ----> else
  ------> // Entra no estado normal
  ------> Checa os sensores e age conforme necessário*
  ```
*Para maiores detalhes, olhar o [Funcionamento do programa](https://github.com/VinPer/IntelliVase#funcionamento-do-programa).

  ### Debug
  ```
  --> Configuração inicial
  --> while (true)
  ----> Exibe os valores lidos de todos os sensores
  ----> Testa o comportamento dos dispositivos
  ```


## Processo de montagem
![Imp1](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/Impermeabilizacao1.jpeg) | ![Imp2](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/Impermeabilizacao2.jpeg)
------------ | ------------
Processo de impermeabilização | Processo de impermeabilização
![Circ](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/CircuitoInterno.jpeg) | ![ApEx1](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/AparenciaExterna1.jpeg)
Circuito interno | Aparência externa
![LEDs](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/LEDs.jpeg) | ![ApEx2](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/AparenciaExterna2.jpeg)
LEDs em funcionamento | Aparência externa
![EmFunc1](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/EmFuncionamento1.jpeg) | ![EmFunc2](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/EmFuncionamento2.jpeg)
IntelliVase em funcionamento | IntelliVase em funcionamento
![EmFunc3](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/EmFuncionamento3.jpeg) | ![EmFunc4](https://raw.githubusercontent.com/VinPer/IntelliVase/master/Imagens/EmFuncionamento4.jpeg)
IntelliVase em funcionamento | IntelliVase em funcionamento

## Referências
[Documentação oficial Mbed OS](https://os.mbed.com/docs/mbed-os/v5.12/introduction/index.html)  
[Biblioteca Adafruit GFX](https://os.mbed.com/components/Adafruit-OLED-128x32/)  
[Biblioteca DS1307](https://os.mbed.com/users/harrypowers/code/DS1307/)
