#include "WB_EEPROM.h"

struct WB_Config EC_Config;

/**
   Инициализация EEPROM
*/
void EC_begin(void) {
  size_t sz1 = sizeof(EC_Config);
  EEPROM.begin(sz1);
  //   Serial.print("\nEEPROM init. Size = "); Serial.print((int)sz1);

}

/**
   Читаем конфигурацию из EEPROM в память
*/
void EC_read(void) {
  size_t sz1 = sizeof(EC_Config);
  for ( int i = 0; i < sz1; i++ ) {
    uint8_t c = EEPROM.read(i);
    *((uint8_t*)&EC_Config + i) = c;
  }
  uint16_t src = EC_SRC();
  if ( EC_Config.SRC == src ) {
    Serial.print("\nEEPROM Config is correct\n");
  }
  else {
    Serial.printf("\nEEPROM SRC is not valid: %d %d\n", src, EC_Config.SRC);
    EC_default();
    EC_save();
  }
}

/**
   Устанавливаем значения конфигурации по-умолчанию
*/
void EC_default(void) {
  size_t sz1 = sizeof(EC_Config);
  memset( &EC_Config, '\0', sz1);
  //   for( int i=0, byte *p = (byte *)&EC_Config; i<sz1; i++, p++)
  //       *p = 0;

  strcpy(EC_Config.ESP_NAME, "WiFi_Homebrewer");
  strcpy(EC_Config.ESP_PASS, "admin");
  strcpy(EC_Config.AP_SSID, "$SSID_NAme");
  strcpy(EC_Config.AP_PASS, "$PASS");
  EC_Config.IP[0]      = 192;
  EC_Config.IP[1]      = 168;
  EC_Config.IP[2]      = 2;
  EC_Config.IP[3]      = 59;
  EC_Config.MASK[0]    = 255;
  EC_Config.MASK[1]    = 255;
  EC_Config.MASK[2]    = 255;
  EC_Config.MASK[3]    = 0;
  EC_Config.GW[0]      = 192;
  EC_Config.GW[1]      = 168;
  EC_Config.GW[2]      = 2;
  EC_Config.GW[3]      = 10;
  EC_Config.Kpid[0]    = 50;       //Kp
  EC_Config.Kpid[1]    = 1;        //Ki
  EC_Config.Kpid[2]    = 15;       //Kd
  EC_Config.Kpid[3]    = 3000;     //Windowsize
  EC_Config.Kpid[4]    = 0;        //Номер текущего рецепта
  EC_Config.StageTmp[0][0]   = 3500;    // MashIn - 35 град. (нужно делить на 100)
  EC_Config.StageTmp[0][1] = 3500; EC_Config.StageTmp[0][2] = 3500; EC_Config.StageTmp[0][3] = 3500; EC_Config.StageTmp[0][4] = 3500;
  EC_Config.StageTmp[0][5] = 3500; EC_Config.StageTmp[0][6] = 3500; EC_Config.StageTmp[0][7] = 3500; EC_Config.StageTmp[0][8] = 3500;
  EC_Config.StageTmp[1][0]   = 4500;    // 1 Пауза - 45 кислотная
  EC_Config.StageTmp[1][1] = 4500; EC_Config.StageTmp[1][2] = 4500; EC_Config.StageTmp[1][3] = 4500 ; EC_Config.StageTmp[1][4] = 4500;
  EC_Config.StageTmp[1][5] = 4500; EC_Config.StageTmp[1][6] = 4500; EC_Config.StageTmp[1][7] = 4500 ; EC_Config.StageTmp[1][8] = 4500;
  EC_Config.StageTmp[2][0]   = 5200;    // 2 Пауза - 52 белковая
  EC_Config.StageTmp[2][1] = 5200; EC_Config.StageTmp[2][2] = 5200; EC_Config.StageTmp[2][3] = 5200 ; EC_Config.StageTmp[2][4] = 5200;
  EC_Config.StageTmp[2][5] = 5200; EC_Config.StageTmp[2][6] = 5200; EC_Config.StageTmp[2][7] = 5200 ; EC_Config.StageTmp[2][8] = 5200;
  EC_Config.StageTmp[3][0]   = 6200;    // 3 Пауза - 62
  EC_Config.StageTmp[3][1] = 6200; EC_Config.StageTmp[3][2] = 6200; EC_Config.StageTmp[3][3] = 6200 ; EC_Config.StageTmp[3][4] = 6200;
  EC_Config.StageTmp[3][5] = 6200; EC_Config.StageTmp[3][6] = 6200; EC_Config.StageTmp[3][7] = 6200 ; EC_Config.StageTmp[3][8] = 6200;
  EC_Config.StageTmp[4][0]   = 6400;   // 4 Пауза - 64
  EC_Config.StageTmp[4][1] = 6400; EC_Config.StageTmp[4][2] = 6400; EC_Config.StageTmp[4][3] = 6400 ; EC_Config.StageTmp[4][4] = 6400;
  EC_Config.StageTmp[4][5] = 6400; EC_Config.StageTmp[4][6] = 6400; EC_Config.StageTmp[4][7] = 6400 ; EC_Config.StageTmp[4][8] = 6400;
  EC_Config.StageTmp[5][0]   = 6600;   // 5 Пауза - 66
  EC_Config.StageTmp[5][1] = 6600; EC_Config.StageTmp[5][2] = 6600; EC_Config.StageTmp[5][3] = 6600 ; EC_Config.StageTmp[5][4] = 6600;
  EC_Config.StageTmp[5][5] = 6600; EC_Config.StageTmp[5][6] = 6600; EC_Config.StageTmp[5][7] = 6600 ; EC_Config.StageTmp[5][8] = 6600;
  EC_Config.StageTmp[6][0]   = 6800;   // 6 Пауза - 68
  EC_Config.StageTmp[6][1] = 6800; EC_Config.StageTmp[6][2] = 6800; EC_Config.StageTmp[6][3] = 6800 ; EC_Config.StageTmp[6][4] = 6800;
  EC_Config.StageTmp[6][5] = 6800; EC_Config.StageTmp[6][6] = 6800; EC_Config.StageTmp[6][7] = 6800 ; EC_Config.StageTmp[6][8] = 6800;
  EC_Config.StageTmp[7][0]   = 7200;   // 7 Пауза - 72
  EC_Config.StageTmp[7][1] = 7200; EC_Config.StageTmp[7][2] = 7200; EC_Config.StageTmp[7][3] = 7200 ; EC_Config.StageTmp[7][4] = 7200;
  EC_Config.StageTmp[7][5] = 7200; EC_Config.StageTmp[7][6] = 7200; EC_Config.StageTmp[7][7] = 7200 ; EC_Config.StageTmp[7][8] = 7200;
  EC_Config.StageTmp[8][0]   = 7800;   // MashOut - 78
  EC_Config.StageTmp[8][1] = 7800; EC_Config.StageTmp[8][2] = 7800; EC_Config.StageTmp[8][3] = 7800 ; EC_Config.StageTmp[8][4] = 7800;
  EC_Config.StageTmp[8][5] = 7800; EC_Config.StageTmp[8][6] = 7800; EC_Config.StageTmp[8][7] = 7800 ; EC_Config.StageTmp[8][8] = 7800;

  EC_Config.StageTim[0][0]   = 5;    // MashIn
  EC_Config.StageTim[0][1] = 5; EC_Config.StageTim[0][2] = 5; EC_Config.StageTim[0][3] = 5; EC_Config.StageTim[0][4] = 5;
  EC_Config.StageTim[0][5] = 5; EC_Config.StageTim[0][6] = 5; EC_Config.StageTim[0][7] = 5; EC_Config.StageTim[0][8] = 5;
  EC_Config.StageTim[1][0]   = 0;    // 1 Пауза  кислотная
  EC_Config.StageTim[1][1] = 0; EC_Config.StageTim[1][2] = 0; EC_Config.StageTim[1][3] = 5; EC_Config.StageTim[1][4] = 5;
  EC_Config.StageTim[1][5] = 0; EC_Config.StageTim[1][6] = 0; EC_Config.StageTim[1][7] = 0; EC_Config.StageTim[1][8] = 0;
  EC_Config.StageTim[2][0]   = 0;    // 2 Пауза  белковая
  EC_Config.StageTim[2][1] = 0; EC_Config.StageTim[2][2] = 0; EC_Config.StageTim[2][3] = 10; EC_Config.StageTim[2][4] = 10;
  EC_Config.StageTim[2][5] = 0; EC_Config.StageTim[2][6] = 0; EC_Config.StageTim[2][7] = 0; EC_Config.StageTim[2][8] = 0;
  EC_Config.StageTim[3][0]   = 0;    // 3 Пауза
  EC_Config.StageTim[3][1] = 0; EC_Config.StageTim[3][2] = 0; EC_Config.StageTim[3][3] = 0; EC_Config.StageTim[3][4] = 0;
  EC_Config.StageTim[3][5] = 0; EC_Config.StageTim[3][6] = 0; EC_Config.StageTim[3][7] = 0; EC_Config.StageTim[3][8] = 0;
  EC_Config.StageTim[4][0]   = 0;    // 4 Пауза
  EC_Config.StageTim[4][1] = 0; EC_Config.StageTim[4][2] = 0; EC_Config.StageTim[4][3] = 0; EC_Config.StageTim[4][4] = 0;
  EC_Config.StageTim[4][5] = 0; EC_Config.StageTim[4][6] = 0; EC_Config.StageTim[4][7] = 0; EC_Config.StageTim[4][8] = 0;
  EC_Config.StageTim[5][0]   = 0;   // 5 Пауза
  EC_Config.StageTim[5][1] = 0; EC_Config.StageTim[5][2] = 0; EC_Config.StageTim[5][3] = 60; EC_Config.StageTim[5][4] = 60;
  EC_Config.StageTim[5][5] = 0; EC_Config.StageTim[5][6] = 0; EC_Config.StageTim[5][7] = 0; EC_Config.StageTim[5][8] = 0;
  EC_Config.StageTim[6][0]   = 60;   // 6 Пауза
  EC_Config.StageTim[6][1] = 0; EC_Config.StageTim[6][2] = 0; EC_Config.StageTim[6][3] = 0; EC_Config.StageTim[6][4] = 0;
  EC_Config.StageTim[6][5] = 0; EC_Config.StageTim[6][6] = 0; EC_Config.StageTim[6][7] = 0; EC_Config.StageTim[6][8] = 0;
  EC_Config.StageTim[7][0]   = 20;   // 7 Пауза
  EC_Config.StageTim[7][1] = 60; EC_Config.StageTim[7][2] = 60; EC_Config.StageTim[7][3] = 10; EC_Config.StageTim[7][4] = 10;
  EC_Config.StageTim[7][5] = 60; EC_Config.StageTim[7][6] = 60; EC_Config.StageTim[7][7] = 0; EC_Config.StageTim[7][8] = 0;
  EC_Config.StageTim[8][0]   = 10;   // MashOut
  EC_Config.StageTim[8][1] = 10; EC_Config.StageTim[8][2] = 10; EC_Config.StageTim[8][3] = 10; EC_Config.StageTim[8][4] = 10;
  EC_Config.StageTim[8][5] = 10; EC_Config.StageTim[8][6] = 10; EC_Config.StageTim[8][7] = 10; EC_Config.StageTim[8][8] = 10;
  EC_Config.StageTim[9][0]   = 90;   // Время кипячения
  EC_Config.StageTim[9][1] = 90; EC_Config.StageTim[9][2] = 90; EC_Config.StageTim[9][3] = 61; EC_Config.StageTim[9][4] = 61;
  EC_Config.StageTim[9][5] = 90; EC_Config.StageTim[9][6] = 90; EC_Config.StageTim[9][7] = 90; EC_Config.StageTim[9][8] = 90;
  EC_Config.BoilTmp       = 10100; // Температура кипячения *100
  EC_Config.Autobrew      = 0;    // Режим авто запущен/нет
  EC_Config.StageN        = 0;    // Номер текущей Т-паузы
  EC_Config.StageT        = 0;    // Время текущей Т-паузы
  EC_Config.HopN          = 0;    // Номер текущего вноса хмеля (0 - 1 - 2 - 3 - 4)
  EC_Config.BoilHopTim[0] = 90;   // Время кипячения
  EC_Config.BoilHopTim[1] = 60;   // Время 1 вноса хмеля
  EC_Config.BoilHopTim[2] = 20;   // Время 2 вноса хмеля
  EC_Config.BoilHopTim[3] = 15;   // Время 3 вноса хмеля
  EC_Config.BoilHopTim[4] = 10;   // Время 4 вноса хмеля
  EC_Config.BoilHopTim[5] = 5;    // Время 5 вноса хмеля
  EC_Config.StageV        = 9;    // Количество Т-пауз вместе с MashIn - 9  без Boil (кипячения)
  EC_Config.HopV          = 6;    // Количество вносов хмеля - 5 + кипячение (c Boil - кипячением)
  EC_Config.StaticIP        = false;    // Статический IP



  EC_save();
}

/**
   Сохраняем значение конфигурации в EEPROM
*/
void EC_save(void) {
  EC_Config.SRC = EC_SRC();
  size_t sz1 = sizeof(EC_Config);
  for ( int i = 0; i < sz1; i++)
    EEPROM.write(i, *((uint8_t*)&EC_Config + i));
  EEPROM.commit();
  Serial.printf("\nSave Config to EEPROM. SCR=%d\n", EC_Config.SRC);
}

/**
   Сохраняем значение конфигурации в EEPROM
*/
uint16_t EC_SRC(void) {
  uint16_t src = 0;
  size_t sz1 = sizeof(EC_Config);
  uint16_t src_save = EC_Config.SRC;
  EC_Config.SRC = 0;
  for ( int i = 0; i < sz1; i++)src += *((uint8_t*)&EC_Config + i);
  //   Serial.print("\nSCR=");Serial.print(src);
  EC_Config.SRC = src_save;
  return src;
}

