#ifndef WB_EEPROM_h
#define WB_EEPROM_h
#include <ESP8266WiFi.h>
#include <EEPROM.h>

extern struct WB_Config EC_Config;

struct WB_Config {
  // Наименование в режиме точки доступа
  char ESP_NAME[32];
  char ESP_PASS[32];
  // Параметры подключения в режиме клиента
  char AP_SSID[32];
  char AP_PASS[32];
  IPAddress IP;
  IPAddress MASK;
  IPAddress GW;
  int16_t Kpid[5];       // Kp, Ki, Kd Windowsize
  uint16_t StageTmp[9][9];  // Температура пауз [номер паузы][номер рецепта]
  uint8_t StageTim[10][9];   // Время пауз [номер паузы][номер рецепта]
  boolean Autobrew;      // Режим авто запущен/нет
  uint8_t StageN;        // Номер текущей Т-паузы
  uint8_t StageT;        // Время текущей Т-паузы
  uint8_t HopN;          // Номер текущего вноса хмеля (0 - 1 - 2 - 3 - 4)
  uint8_t BoilHopTim[6]; // Время вноса хмеля
  uint16_t BoilTmp;      // Температура кипячения
  uint8_t StageV;        // Количество Т-пауз вместе с MashIn без Boil (кипячения)
  uint8_t HopV;          // Количество вносов хмеля (c Boil - кипячением)
  boolean StaticIP;        // Номер текущего рецепта



  // Контрольная сумма
  uint16_t SRC;
};


void     EC_begin(void);
void     EC_read(void);
void     EC_save(void);
uint16_t EC_SRC(void);
void     EC_default(void);




#endif
