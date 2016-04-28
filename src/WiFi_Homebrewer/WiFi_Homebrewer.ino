/**
  Контроллер управления пивоварней. Версия 1.0 WiFi
   2016 Андрей Вохромин
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <OneWire.h>
#include <PID_v1.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <pcf8574_esp.h>
#include "WB_EEPROM.h"
#include "WB_HTTP.h"

/* Initialise the LiquidCrystal library. The default address is 0x27
   and this is a 20 x 4 line display */
LiquidCrystal_I2C lcd(0x27, 20, 4);
OneWire ds(14);
PCF8574 pcf(0x26);
int clockAddress = 0x68;   // Адрес часов DS1307
uint8_t second, seconds, minute, hour, dayOfWeek, day, dayOfMonth, month, year;
// Кнопки    push buttons
uint8_t  Button_up = 7;    // Кнопка "+"
uint8_t  Button_dn = 6;    // Кнопка "-"
uint8_t  Button_prev = 4;  // Кнопка "ПРЕД"
uint8_t  Button_nxt = 5;   // Кнопка "СЛЕД"


// outputs
uint8_t HEAT = 16;   // ТЭН
uint8_t PUMP = 2;    // Насос
uint8_t BUZZ = 12;   // Пищалка


// global variables
uint16_t WindowSize;

uint32_t windowStartTime;
uint32_t start;

double Setpoint, Input, Output, eepromKp, eepromKi, eepromKd;
PID myPID(&Input, &Output, &Setpoint, 100, 20, 5, DIRECT);
//boolean autoLoop = false;
boolean manualLoop = false;
boolean waterAdd = false;
boolean Conv_start = false;
boolean mpump = false;  // Флаг включения насоса
boolean mheat = false;  // Флаг включения ТЭНа
boolean wtBtn = false;
boolean autoEnter = false;
boolean malt;
boolean tempReached = false; // Флаг достижения заданной температуры
boolean pumpRest = false;    // Флаг фильтрации (выкл. насос)
boolean stage_pause = false;
boolean filtr = false;
boolean resume = false;
float mset_temp = 35;
float Temp_c, Temp_c2, stageTemp, pumptempError, Temp_PID;
uint8_t x;
uint8_t  stageTime, hopTime;
float BoilTemp;
boolean BoilTempLoop = true;

uint8_t MODE = 0;
uint8_t mainMenu = 0;  // Режим текущий: 0 - Главное меню
uint8_t pumpTime;
uint8_t data[2];
//uint8_t second;
uint8_t Busy = 0;
uint8_t nmbrStgs = 9;
uint8_t nmbrHops = 5;
uint8_t blhpAddr;
uint8_t hopAdd;

char* stgName[] = {"MashIn ", "Stage 1", "Stage 2", "Stage 3", "Stage 4", "Stage 5", "Stage 6", "Stage 7", "MashOut", "Boil   "}; // Т-Паузы

// Символ °С   (код 0176)
uint8_t degc[8] = {0x8, 0x14, 0x8, 0x3, 0x4, 0x4, 0x3, 0x0};
//  B01000,
//  B10100,
//  B01000,
//  B00011,
//  B00100,
//  B00100,
//  B00011,
//  B00000,
uint8_t Buk_B[8]  = {0x1F, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00}; //  Буква Б
uint8_t Buk_b[8]  = {0x00, 0x00, 0x1F, 0x10, 0x1E, 0x11, 0x1E, 0x00}; //  Буква б
uint8_t Buk_v[8]  = {0x00, 0x00, 0x1E, 0x11, 0x1E, 0x11, 0x1E, 0x00}; //  Буква в
uint8_t Buk_G[8]  = {0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00}; //  Буква Г
uint8_t Buk_g[8]  = {0x00, 0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x00}; //  Буква г
uint8_t Buk_D[8]  = {0x06, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x1F, 0x11}; //  Буква Д
uint8_t Buk_d[8]  = {0x00, 0x00, 0x06, 0x0A, 0x0A, 0x0A, 0x1F, 0x11}; //  Буква д
uint8_t Buk_Z[8]  = {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E, 0x00}; //  Буква З
uint8_t Buk_z[8]  = {0x00, 0x00, 0x0E, 0x11, 0x06, 0x11, 0x0E, 0x00}; //  Буква з
uint8_t Buk_L[8]  = {0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x11, 0x00}; //  Буква Л
uint8_t Buk_l[8]  = {0x00, 0x00, 0x07, 0x09, 0x09, 0x09, 0x11, 0x00}; //  Буква л
uint8_t Buk_I[8]  = {0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00}; //  Буква И
uint8_t Buk_i[8]  = {0x00, 0x00, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00}; //  Буква и
uint8_t Buk_II[8] = {0x15, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00}; //  Буква Й
uint8_t Buk_SH[8] = {0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x1F, 0x00}; //  Буква Ш
uint8_t Buk_sh[8] = {0x00, 0x00, 0x15, 0x15, 0x15, 0x15, 0x1F, 0x00}; //  Буква ш
uint8_t Buk_n[8]  = {0x00, 0x00, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00}; //  Буква н
uint8_t Buk_P[8]  = {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00}; //  Буква П
uint8_t Buk_p[8]  = {0x00, 0x00, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x00}; //  Буква п
uint8_t Buk_t[8]  = {0x00, 0x00, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00}; //  Буква т
uint8_t Buk_MZ[8] = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00}; //  Буква Ь
uint8_t Buk_mz[8] = {0x00, 0x00, 0x10, 0x10, 0x1E, 0x11, 0x1E, 0x00}; //  Буква ь
uint8_t Buk_JA[8] = {0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00}; //  Буква Я
uint8_t Buk_ja[8] = {0x00, 0x00, 0x0F, 0x11, 0x0F, 0x09, 0x11, 0x00}; //  Буква я
uint8_t Buk_J[8]  = {0x15, 0x15, 0x0E, 0x04, 0x0E, 0x15, 0x15, 0x00}; //  Буква Ж
uint8_t Buk_CH[8] = {0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00}; //  Буква Ч
uint8_t Buk_ch[8] = {0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x00}; //  Буква ч
uint8_t Buk_Y[8]  = {0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E, 0x00}; //  Буква У
uint8_t Buk_iii[8] = {0x00, 0x00, 0x11, 0x19, 0x15, 0x15, 0x19, 0x00}; //  Буква ы
uint8_t Buk_III[8] = {0x11, 0x11, 0x19, 0x15, 0x15, 0x15, 0x19, 0x00}; //  Буква ы
uint8_t Buk_JE[8] = {0x0E, 0x11, 0x01, 0x0F, 0x01, 0x11, 0x0E, 0x00}; //  Буква Э
uint8_t Buk_F[8]  = {0x0E, 0x15, 0x15, 0x15, 0x0E, 0x04, 0x04, 0x00}; //  Буква Ф
uint8_t Buk_C[8]  = {0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1F, 0x01}; //  Буква Ц


/*byte Buk_b[8] ={
  B01110,
  B10101,
  B10101,
  B10101,
  B01110,
  B00100,
  B00100,
  B00000,
  };*/


//****** start of the funtions**************

//  ***  Пищать "number" писков ***
void Buzzer(uint8_t number, int time) {
  for (int i = 0; i < number; i++) {
    digitalWrite (BUZZ, HIGH);
    delay (time);
    digitalWrite(BUZZ, LOW);
    delay(100);
  }
}

//  ***  Пауза если кнопка "ПРЕД" нажата больше 1 сек  ***
void pause_stage(void) {
  if (Button_1sec_press(Button_prev) || stage_pause == true) { // если кнопка "ПРЕД" нажата больше 1 сек
    Buzzer(1, 300);                          // 1 писк
    stage_pause = true;
    digitalWrite(HEAT, HIGH);            // выключить ТЭН
    digitalWrite(PUMP, HIGH);            // выключить насос
    lcd.createChar(1, Buk_P); //
    lcd.createChar(2, Buk_z);
    lcd.createChar(3, Buk_d);
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print("       ");    //  Пауза
    lcd.write((uint8_t)1);
    lcd.print("ay");
    lcd.write((uint8_t)2);
    lcd.print("a        ");
    display_lcd(0, 3, "          Ha"); //  Назад
    lcd.write((uint8_t)2);
    lcd.print("a");
    lcd.write((uint8_t)3);
    while (stage_pause)                 // и ждем снова нажатия кнопки "ПРЕД" больше 1 сек
    { if (Button_1sec_press(Button_prev)) stage_pause = false;
      HTTP_loop();
      delay(100);
    }
  }
}

//************ нажатие кнопки больше 1 сек)
boolean Button_1sec_press (uint8_t Button_press) {
  if (pcf.read(Button_press) == 0) {
    delay (1000);
    if (pcf.read(Button_press) == 0) {
      Buzzer(1, 150);                          // 1 короткий писк
      while (pcf.read(Button_press) == 0) {
        HTTP_loop();
        delay(100);
      }
      return 1;
    }
  }
  return 0;
}

// ************ Нажатие кнопки
//repeat button press
boolean Button_repeat (uint8_t Button_press) {
  if (pcf.read(Button_press) == 0) {
    Buzzer(1, 50);                          // 1 короткий писк
    return 1;
  }
  return 0;
}

//  **** Ожидание подтверждения. Кнопка ПРЕД. - ДА, СЛЕД - НЕТ
boolean wait_for_confirm (boolean& test)
{
  wtBtn = true;
  while (wtBtn) {              // wait for comfirmation
    if (Button_hold_press(Button_prev)) {
      test = true;
      wtBtn = false;
      //      lcd.clear();
    }
    if (Button_hold_press(Button_nxt)) {
      test = false;
      wtBtn = false;
    }
    HTTP_loop();
    delay(100);
  }
  lcd.clear();
}

// ************ Нажатие и отпускание кнопки
// holds whilst button pressed
boolean Button_hold_press (uint8_t Button_press) {
  if (pcf.read (Button_press) == 0) {
    Buzzer(1, 50);                          // 1 короткий писк
    while (pcf.read (Button_press) == 0) {
      HTTP_loop();
      delay(100);
    }
    return 1;
  }
  return 0;
}
//************  Вывод на экран  ***
void display_lcd (uint8_t pos , uint8_t line , const char* lable) {
  lcd.setCursor(pos, line);
  lcd.print(lable);
}

//  ************ чтение температуры из DS18B20
// reads the DS18B20 temerature probe
void Temperature(void) {
  ds.reset();
  ds.skip();
  // start conversion and return
  if (!(Conv_start)) {
    ds.write(0x44, 0);
    Conv_start = true;
    return;
  }
  // check for conversion if it isn't complete return if it is then convert to decimal
  if (Conv_start) {
    Busy = ds.read_bit();
    if (Busy == 0) {
      return;
    }
    ds.reset();
    ds.skip();
    ds.write(0xBE);
    for ( int i = 0; i < 2; i++) {           // we need 2 bytes
      data[i] = ds.read();
    }
    unsigned int raw = (data[1] << 8) + data[0];
    Temp_PID = (raw & 0xFFFF) * 0.0625;
    Temp_c = (raw & 0xFFFC) * 0.0625;
    Conv_start = false;
    return;
  }
}
//  ************  ПИД-контроллер
void PID_HEAT (void) {
  if (autoEnter) {
    Setpoint = stageTemp;
  }
  else {
    Setpoint = mset_temp;
  }
  Input = Temp_PID;
  if ((Setpoint - Input) > 5) {
    digitalWrite(HEAT, LOW);
    if ((Setpoint - Input) < 6) {
      myPID.Compute();
    }
  }
  else {
    myPID.Compute();
    unsigned long now = millis();
    if ((now - windowStartTime) > WindowSize) {
      windowStartTime += WindowSize;  //time to shift the Relay Window
    }
    if ((Output * (WindowSize / 100)) > (now - windowStartTime)) {
      digitalWrite(HEAT, LOW);
    }
    else {
      digitalWrite(HEAT, HIGH);
    }
  }
}



//  ************  Загрузка установок для ПИД-контроллера
void load_pid_settings (void)
{
  eepromKp = EC_Config.Kpid[0];// read the PID settings from the EEPROM
  eepromKi = EC_Config.Kpid[1];
  eepromKd = EC_Config.Kpid[2];
  eepromKi = eepromKi / 100;
  myPID.SetTunings(eepromKp, eepromKi, eepromKd); // send the PID settings to the PID
  WindowSize = EC_Config.Kpid[3];
  myPID.SetOutputLimits(0, 100);
  myPID.SetSampleTime(5000);
}


// ***    Проверка на ВЫХОД из режима  (одновременное нажатие + и -)
void quit_mode (boolean& processLoop)
{
  if ((pcf.read(Button_dn) == 0) && (pcf.read(Button_up) == 0)) {
    digitalWrite(HEAT, HIGH);
    digitalWrite(PUMP, HIGH);
    processLoop = false;
    lcd.clear();
    Buzzer(1, 150);
  }
}


//  ************  Изменение установленной температуры
float change_temp(float& temp_change, int upper_limit, int lower_limit)
{
  // Increase set temp
  if (Button_repeat(Button_up)) { // кн. - ВВЕРХ - увеличить температуру
    //    if (temp_change>=100){       // после 100 гр  по 1 гр
    //      temp_change++;
    //    }
    //    else{
    temp_change += 0.25;         // а до - по 0,25
    //    }
  }
  // decrease temp
  if (Button_repeat(Button_dn)) { // кн. - ВНИЗ - уменьшить температуру
    //    if(temp_change>100){
    //      temp_change--;
    //    }
    //    else{
    temp_change -= 0.25;
    //    }
  }
  if (temp_change > upper_limit)temp_change = upper_limit;
  if ( temp_change < lower_limit) temp_change = lower_limit;
}

int change_set(int& set_change, int upper_limit, int lower_limit, int step_size)
{
  // Increase set temp
  if (Button_repeat(Button_up)) {
    set_change += step_size;
    //    display_lcd(0,1,"                    ");
  }
  if (set_change > upper_limit)set_change = upper_limit;

  // decrease temp
  if (Button_repeat(Button_dn))
  {
    set_change -= step_size;
    //    display_lcd(0,1,"                    ");
  }
  if ( set_change < lower_limit) set_change = lower_limit;
}


int change_set(byte& set_change, int upper_limit, int lower_limit, int step_size)
{
  // Increase set temp
  if (Button_repeat(Button_up)) {
    set_change += step_size;
    //    display_lcd(0,1,"                    ");
  }
  if (set_change > upper_limit)set_change = upper_limit;

  // decrease temp
  if (Button_repeat(Button_dn))
  {
    set_change -= step_size;
    //    display_lcd(0,1,"                    ");
  }
  if ( set_change < lower_limit) set_change = lower_limit;
}



// *** Настройка ПИД параметров
void unit_set (void)
{
  int param[] = {100, -100, 1, 100, -100, 1, 100, -100, 1, 5000, 500, 500, 8, 0, 1};
  uint8_t a = 0;
  boolean pidLoop = false;
  int pidSet;
  char* setName[] = {"Kp = ", "Ki = ", "Kd = ", "Windowsize = ", "Receipt = "};

  for (uint8_t i = 0; i < 5; i++) {
    pidSet = EC_Config.Kpid[i];
    pidLoop = true;
    display_lcd(0, 1, "                    ");
    display_lcd(0, 2, "                    ");
    display_lcd(0, 3, " -     +        Next");
    while (pidLoop) {
      display_lcd(0, 1, setName[i]);
      lcd.print(pidSet);
      lcd.print("   ");
      change_set(pidSet, param[a], param[a + 1], param[a + 2]); // Проверка на редактирование
      quit_mode(pidLoop);
      if (!(pidLoop))i = 5;
      if (Button_hold_press(Button_nxt)) {
        EC_Config.Kpid[i] = pidSet;
        EC_save();
        pidLoop = false;
      }
      HTTP_loop();
      delay(100);
    }
    a += 3;
  }
}

// *** Настройка температурных Т-пауз
void set_stages (void)
{ boolean autotempLoop = false;
  boolean autotimeLoop = false;
  float stgtmpSet;
  int stgtimSet;
  //  nmbrStgs = EC_Config.StageV;

  for (int i = 0; i < EC_Config.StageV; i++) { // Настраиваем температуру пауз
    stgtmpSet = EC_Config.StageTmp[i][EC_Config.Kpid[4]];
    stgtmpSet = stgtmpSet / 100;
    autotempLoop = true;
    lcd.createChar(1, Buk_p);  // п
    lcd.createChar(2, Buk_t);  // т
    lcd.createChar(3, Buk_z);  // з
    lcd.createChar(4, Buk_iii); // ы
    display_lcd(0, 1, "                    ");
    display_lcd(0, 2, "                    ");
    display_lcd(0, 3, " -     +        Next");
    while (autotempLoop) { // Настраиваем температуру пауз
      lcd.setCursor(0, 1);
      lcd.print("Tem");            // Тем
      lcd.write((uint8_t)1);       // п
      lcd.print("epa");            // ера
      lcd.write((uint8_t)2);       // т
      lcd.print("ypa ");           // ура
      if (i > 0 && i < (EC_Config.StageV - 1)) {
        lcd.write((uint8_t)1);       //  п
        lcd.print("ay");             // ау
        lcd.write((uint8_t)3);       // з
        lcd.write((uint8_t)4);       // ы
        lcd.print(" ");
        lcd.print(i);                // номер паузы
      }

      else {
        lcd.print(" ");
        lcd.print(stgName[i]);
      }
      lcd.setCursor(0, 2);
      lcd.print("     = ");
      lcd.print(stgtmpSet);
      lcd.print(" ");
      lcd.write((uint8_t)0);
      lcd.print("      ");

      quit_mode(autotempLoop);             // Проверка на выход
      if (autotempLoop == false) {
        return;  // Выход без сохранения если нажаты 2 кнопки выхода
      }
      change_temp(stgtmpSet, 85, 20);                      //Проверка на редактирование температуры, мах=85, min=20
      if (Button_hold_press(Button_nxt)) {                 // Если нажата кнопка ДАЛЬШЕ
        EC_Config.StageTmp[i][EC_Config.Kpid[4]] = stgtmpSet * 100;
        EC_save();                                      // Запись установленной температуры и переход к редактированию времени паузы
        display_lcd(0, 1, "                    ");
        display_lcd(0, 2, "                    ");
        display_lcd(0, 3, " -     +        Next");
        autotempLoop = false;
      }
      HTTP_loop();
      delay(100);
    }
    autotimeLoop = true; // Настраиваем время пауз
    stgtimSet = EC_Config.StageTim[i][EC_Config.Kpid[4]];
    lcd.createChar(1, Buk_ja);  // я
    lcd.createChar(2, Buk_p);   // п
    lcd.createChar(3, Buk_z);   // з
    lcd.createChar(4, Buk_iii); // ы
    lcd.createChar(5, Buk_i);   // и
    lcd.createChar(6, Buk_n);   // н
    display_lcd(0, 1, "                    ");
    display_lcd(0, 2, "                    ");
    display_lcd(0, 3, " -     +        Next");
    while (autotimeLoop) { // Настраиваем время пауз
      lcd.setCursor(0, 1);     //Время MashIn
      lcd.print("   Bpem");    // Врем
      lcd.write((uint8_t)1);   // я
      lcd.print(" ");          //
      if (i > 0 && i < (EC_Config.StageV - 1)) {
        lcd.write((uint8_t)2);
        lcd.print("ay");
        lcd.write((uint8_t)3);
        lcd.write((uint8_t)4);
        lcd.print(" ");
        lcd.print(i);
      }

      else lcd.print(stgName[i]);   // Имя паузы
      lcd.setCursor(0, 2);
      lcd.print("     = ");
      lcd.print(stgtimSet);    // Время паузы
      lcd.print(" m");         // м
      lcd.write((uint8_t)5);   // и
      lcd.write((uint8_t)6);   // н
      lcd.print("       ");
      quit_mode(autotimeLoop);                 // Проверка на выход
      if (autotimeLoop == false) {
        return;  // Выход без сохранения если нажаты 2 кнопки выхода
      }
      change_set(stgtimSet, 120, 0, 1);        //Проверка на редактирование времени, мах=120, min=0
      if (Button_hold_press(Button_nxt)) {     // Если нажата кнопка ДАЛЬШЕ
        EC_Config.StageTim[i][EC_Config.Kpid[4]] = stgtimSet;
        EC_save();                         // Запись установленной температуры и переход к редактированию времени паузы
        display_lcd(0, 1, "                    ");
        display_lcd(0, 2, "                    ");
        display_lcd(0, 3, " -     +        Next");
        autotimeLoop = false;
      }
      HTTP_loop();
      delay(100);
    }
  }
  boil_temp_loop ();
}

// ***  Настройка температуры кипячения
void boil_temp_loop (void)
{
  BoilTemp = EC_Config.BoilTmp;
  BoilTemp = BoilTemp / 100;
  lcd.createChar(1, Buk_p);
  lcd.createChar(2, Buk_t);
  lcd.createChar(3, Buk_i);
  lcd.createChar(4, Buk_ja);
  lcd.createChar(5, Buk_ch);
  lcd.createChar(6, Buk_n);
  display_lcd(0, 1, "                    ");
  display_lcd(0, 2, "                    ");
  display_lcd(0, 3, " -     +        Next");
  while (BoilTempLoop) {
    lcd.setCursor(0, 1);
    lcd.print("    Tem");            //Температура кипячения
    lcd.write((uint8_t)1);
    lcd.print("epa");
    lcd.write((uint8_t)2);
    lcd.print("ypa ");
    lcd.setCursor(0, 2);
    lcd.print("k");
    lcd.write((uint8_t)3);
    lcd.write((uint8_t)1);
    lcd.write((uint8_t)4);
    lcd.write((uint8_t)5);
    lcd.print("e");
    lcd.write((uint8_t)6);
    lcd.write((uint8_t)3);
    lcd.write((uint8_t)4);
    lcd.print(" = ");
    lcd.print(BoilTemp);
    lcd.write((uint8_t)0);
    lcd.print(" ");
    quit_mode(BoilTempLoop);
    if (BoilTempLoop == false) {
      return;
    }
    change_temp(BoilTemp, 105, 95); //******************
    if (Button_hold_press(Button_nxt)) {
      EC_Config.BoilTmp = BoilTemp * 100;
      EC_save();                         // Запись установленной температуры и переход к редактированию времени паузы
      display_lcd(0, 1, "                    ");
      display_lcd(0, 2, "                    ");
      display_lcd(0, 3, " -     +        Next");
      BoilTempLoop = false;
    }
    HTTP_loop();
    delay(100);
  }
  set_hops();
}

// *** Настройка кипячения и закладки хмеля
void set_hops (void)
{
  boolean hopLoop = false;
  int hopSet;
  int boilSet = 90;

  nmbrHops = EC_Config.HopV;
  nmbrHops += 1;

  lcd.createChar(1, Buk_ja);
  lcd.createChar(2, Buk_i);
  lcd.createChar(3, Buk_p);
  lcd.createChar(4, Buk_ch);
  lcd.createChar(5, Buk_n);
  lcd.createChar(6, Buk_v);
  lcd.createChar(7, Buk_l);
  display_lcd(0, 1, "                    ");
  display_lcd(0, 2, "                    ");
  for (int i = 0; i < EC_Config.HopV; i++) {
    hopLoop = true;
    if (i == 0) hopSet = EC_Config.StageTmp[9][EC_Config.Kpid[4]];
    else hopSet = EC_Config.BoilHopTim[i];
    while (hopLoop) {
      if (i == 0) {
        lcd.setCursor(0, 1);
        lcd.print("  Bpem");            //Время кипячения
        lcd.write((uint8_t)1);
        lcd.print(" ");
        lcd.print("k");
        lcd.write((uint8_t)2);
        lcd.write((uint8_t)3);
        lcd.write((uint8_t)1);
        lcd.write((uint8_t)4);
        lcd.print("e");
        lcd.write((uint8_t)5);
        lcd.write((uint8_t)2);
        lcd.write((uint8_t)1);
        lcd.setCursor(0, 2);
        lcd.print("     = ");
        lcd.print(hopSet);
        lcd.print(" m");
        lcd.write((uint8_t)2);
        lcd.write((uint8_t)5);
        lcd.print("       ");
        boilSet = hopSet;
      }
      else {
        lcd.createChar(4, Buk_z);  //Время вноса хмеля
        lcd.setCursor(0, 1);
        lcd.print("   ");
        lcd.print(i);
        lcd.print(" ");
        lcd.write((uint8_t)6);
        lcd.write((uint8_t)5);
        lcd.print("oc xme");
        lcd.write((uint8_t)7);
        lcd.write((uint8_t)1);
        lcd.setCursor(0, 2);
        lcd.print("     ");
        lcd.write((uint8_t)4);
        lcd.print("a ");
        lcd.print(hopSet);
        lcd.print(" m");
        lcd.write((uint8_t)2);
        lcd.write((uint8_t)5);
        lcd.print("    ");


      }
      quit_mode(hopLoop);
      if ( hopLoop == false) {
        return;
      }
      if (i == 0) change_set(hopSet, 180, 0, 1);
      else change_set(hopSet, boilSet, 0, 1);
      if (Button_hold_press(Button_nxt)) {
        if (i == 0) EC_Config.StageTmp[9][EC_Config.Kpid[4]] = hopSet;
        else EC_Config.BoilHopTim[i] = hopSet;
        EC_save();                         // Запись установленной температуры и переход к редактированию времени паузы
        display_lcd(0, 1, "                    ");
        display_lcd(0, 2, "                    ");
        display_lcd(0, 3, " -     +        Next");
        hopLoop = false;
      }
      HTTP_loop();
      delay(100);
    }
  }
}

void auto_set(void)
{
  set_stages();
}

void setup_mode (void)
{
  byte setupMenu = 0;
  boolean setupLoop = true;
  while (setupLoop) {
    switch (setupMenu) { // to select between PID and Auto menu
      case (0):
        display_lcd(0, 0, "  Unit Parameters ");
        display_lcd(0, 1, "                    ");
        display_lcd(0, 2, "          Auto      ");
        display_lcd(0, 3, " --Exit--      Enter");
        quit_mode(setupLoop);
        if (Button_hold_press(Button_prev))setupMenu = 1;
        if (Button_hold_press(Button_nxt))unit_set();
        break;

      case (1):
        display_lcd(0, 0, "  Auto Parameters");
        display_lcd(0, 1, "                    ");
        display_lcd(0, 2, "          Unit      ");
        display_lcd(0, 3, " --Exit--      Enter");
        quit_mode(setupLoop);
        if (Button_hold_press(Button_prev))setupMenu = 0;
        if (Button_hold_press(Button_nxt))auto_set();
        break;
    }
    HTTP_loop();
    delay(100);
  }
}

//  Установки по умолчанию
void default_set() {
  boolean def;
  lcd.createChar(1, Buk_b); //  Вывод Установки по умолчанию  Сброс установок
  lcd.createChar(2, Buk_t);
  lcd.createChar(3, Buk_n);
  lcd.createChar(4, Buk_v);
  lcd.createChar(5, Buk_D);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  C");       // С
  lcd.write((uint8_t)1);  // б
  lcd.print("poc yc");    // рос ус
  lcd.write((uint8_t)2);  // т
  lcd.print("a");         // а
  lcd.write((uint8_t)3);  // н
  lcd.print("o");         // о
  lcd.write((uint8_t)4);  // в
  lcd.print("ok?");       // ок
  lcd.setCursor(0, 3);
  lcd.print("            ");
  lcd.write((uint8_t)5);  // Д
  lcd.print("a   He");    // а Не
  lcd.write((uint8_t)2);  // т
  wait_for_confirm(def); //  Ожидание подтверждения или отмены
  if (def == true) {
    EC_default();
  }
}
void manual_mode (void)
{ load_pid_settings();
  manualLoop = true;
  while (manualLoop) {           // manual loop
    Temperature();
    lcd.createChar(1, Buk_CH); //  Вывод РУЧНОЙ РЕЖИМ
    lcd.createChar(2, Buk_II);
    lcd.createChar(3, Buk_J);
    lcd.createChar(4, Buk_I);
    lcd.createChar(5, Buk_Y);
    lcd.createChar(6, Buk_t);
    lcd.createChar(7, Buk_JE);
    lcd.setCursor(0, 0);
    lcd.print("   P");       // Р
    lcd.write((uint8_t)5);   // У
    lcd.write((uint8_t)1);   // Ч
    lcd.print("HO");         // НО
    lcd.write((uint8_t)2);   // Й
    lcd.print("  PE");       // РЕ
    lcd.write((uint8_t)3);   // Ж
    lcd.write((uint8_t)4);   // И
    lcd.print("M");          // М
    display_lcd(0, 1, "  Tek. t = ");
    lcd.print(Temp_c);
    lcd.write((uint8_t)0);
    lcd.setCursor(0, 2);
    lcd.print("  ");
    lcd.write((uint8_t)5);   // У
    lcd.print("c");          // с
    lcd.write((uint8_t)6);   // т
    lcd.print(". t = ");     // . t
    lcd.print(mset_temp);    //
    lcd.write((uint8_t)0);
    display_lcd(0, 3, "- Exit +   T");
    lcd.write((uint8_t)7);
    lcd.print("H Hacoc");
    change_temp(mset_temp, 120, 20);
    quit_mode(manualLoop);
    if (!manualLoop) {
      mainMenu = 0;
    }// return;}
    heat_control();
    pump_control();
    if (mheat) {
      PID_HEAT();
    }
    //   Serial.print("\nmainMenu = "); Serial.print(mainMenu);
    HTTP_loop();
    if (mainMenu == 0) return;
    delay(100);
  }
}
void heat_control(void)
{
  //turns heat on or off
  if (Button_hold_press(Button_prev)) {
    if (mheat == false) {
      mheat = true;
      windowStartTime = millis();
    }
    else {
      mheat = false;
      digitalWrite(HEAT, HIGH);
    }
  }
}

void pump_control(void)
{
  //turns the pump on or off
  if (Button_hold_press(Button_nxt)) {
    if (mpump == false) {
      mpump = true;
      digitalWrite(PUMP, LOW);
    }
    else {
      mpump = false;
      digitalWrite(PUMP, HIGH);
    }
  }
}

//  ************  запуск таймера
void start_time (void)
{
  setDateDs1307();
  seconds = 0;
}

void stage_timing (int stage)
{ seconds = second;
  getDateDs1307();
  //        String s = getTimeStr();  Serial.println(s);
  if ((second - seconds) != 0) {                                   // Если тикнула секунда, то...
    if (!(tempReached)) {
      setDateDs1307();  // Обнуляем часы если Т не достигнута
    }
    if (seconds > second) {
      pumpTime++;        // Если прошла минута, то...
      if (stage == 0) pumpTime = 0;  // На MashIn фильтрация отключена
      stageTime--;                   // Минута прошла
      EC_Config.StageT = stageTime;  // Запись точки восстановления
      EC_save();                     // Запись текущего времени процесса
    }
  }
}

//  ************   ВОДА НАЛИТА?
void prompt_for_water (void) {
  MODE = 2;
  HTTP_loop();
  lcd.createChar(1, Buk_D); //  Вывод ВОДА НАЛИТА?
  lcd.createChar(2, Buk_L);
  lcd.createChar(3, Buk_I);
  lcd.createChar(4, Buk_v);
  lcd.createChar(5, Buk_iii);
  lcd.createChar(6, Buk_d);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("    BO");     //  ВОДА НАЛИТА?
  lcd.write((uint8_t)1);
  lcd.print("A HA");
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)3);
  lcd.print("TA?");

  Buzzer(3, 500);
  display_lcd(0, 3, "           ");
  lcd.write((uint8_t)1);
  lcd.print("A  B");
  lcd.write((uint8_t)5);
  lcd.print("xo");
  lcd.write((uint8_t)6);
}

//  ************  Прокачка помпы
void pump_prime(void)
{
  lcd.createChar(1, Buk_P); //  Вывод Прокачка помпы
  lcd.createChar(2, Buk_ch);
  lcd.createChar(3, Buk_p);
  lcd.createChar(4, Buk_iii);
  MODE = 4;
  mpump = false;
  lcd.clear();
  for (int i = 0; i < 10; i++) {
    if (mpump) {
      digitalWrite(PUMP, HIGH);
      mpump = false;
    }
    else {
      digitalWrite(PUMP, LOW);
      mpump = true;
    }
    for (int k = 0; k < 2; k++) {
      lcd.setCursor(0, 1);
      lcd.print("   ");
      lcd.write((uint8_t)1);
      lcd.print("poka");          // Прока
      lcd.write((uint8_t)2);      // ч
      lcd.print("ka ");           // ка
      lcd.write((uint8_t)3);      // п
      lcd.print("om");            // ом
      lcd.write((uint8_t)3);      // п
      lcd.write((uint8_t)4);      // ы
      delay (700);  // Задержка с миганием надписи
      lcd.clear();
      delay (300);
      HTTP_loop();
    }
  }
  lcd.clear();
}

//  ************   Остановка помпы на фильтрацию
void pump_rest (int stage)
{
  if (stage == EC_Config.StageV) { // Если кипячение
    pumpRest = false;
    if (Temp_c < 90.0) digitalWrite(PUMP, LOW); //  Если Т >= 90 град. помпу выключить
    else digitalWrite(PUMP, HIGH);
    if (Temp_c >= (stageTemp - 5)) tempReached = true; // Если Т достигла (Т кипячения-5)
    //        else tempReached = false;
  }
  else {    // если не кипячение
    pumptempError = stageTemp - Temp_c;
    if (pumptempError <= 0) tempReached = true;  //  Проверка достигнута ли заданная температура -0.25
    //            else tempReached = false;
    if ((pumpTime < 10)) { // starts pumps and heat
      digitalWrite(PUMP, LOW);
      pumpRest = false;
    }
    if ((pumpTime >= 10)) { // pump rest  *****  5
      digitalWrite(PUMP, HIGH);
      digitalWrite(HEAT, HIGH);
      if ( !filtr ) {
        Buzzer(1, 500);
        filtr = true;
      }
      pumpRest = true;
    }
    if (pumpTime >= 12 || (pumptempError > 1.0)) {
      filtr = false;  // Если 2 мин простоя прошло или Т упала больше чем на 1 гр - включить помпу
      pumpTime = 0;
    }

  }
}

//  ************  был ли запущен режим АВТО
void check_for_resume(void) {
  if (EC_Config.Autobrew) { // Проверить, был ли запущен режим АВТО, и если ДА, то спросить ПРОДОЛЖИТЬ?
    MODE = 1; // Продолжить варку?
    lcd.createChar(1, Buk_P); //
    lcd.createChar(2, Buk_D);
    lcd.createChar(3, Buk_L);
    lcd.createChar(4, Buk_J);
    lcd.createChar(5, Buk_I);
    lcd.createChar(6, Buk_MZ);
    lcd.createChar(7, Buk_Y);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("  ");         //  ПРОДОЛЖИТЬ ВАРКУ?
    lcd.write((uint8_t)1);
    lcd.print("PO");
    lcd.write((uint8_t)2);
    lcd.print("O");
    lcd.write((uint8_t)3);
    lcd.write((uint8_t)4);
    lcd.write((uint8_t)5);
    lcd.print("T");
    lcd.write((uint8_t)6);
    lcd.print(" BAPK");
    lcd.write((uint8_t)7);
    lcd.print("?");
    lcd.setCursor(0, 3);
    lcd.print("            ");
    lcd.write((uint8_t)2);  // Д
    lcd.print("A   HET");    // A   HET

    wait_for_confirm(resume);
    if (resume == true) {
      x = EC_Config.StageN;   //  Номер текущей Т-паузы
      autoEnter = true;
      lcd.clear();
    }
  }
}

void stage_loop (int stage, float H_temp = 80, float L_temp = 20) {
  int lastminute, stageTime2, pumpTime2;
  boolean addmalt = false;
  while ((stageTime > 0) && (autoEnter)) {
    lastminute = stageTime;
    stage_timing(stage);
    do {
      Temp_c2 = Temp_c;
      Temperature();             //    Считать температуру     get temp
    } while (Temp_c2 != Temp_c);
    pump_rest(stage);
    if (pumpRest == false)PID_HEAT();
    if ( stage == 0 && autoEnter && tempReached &&  !addmalt) { // Добавить солод если Т достигнута на MashIn
      add_malt();
      if (!(autoEnter))break;
      addmalt = true;
      setDateDs1307();  // Обнуляем таймер
    }
    if (stage == EC_Config.StageV) {
      hop_add();
    }
    if (pumpRest) {            //    Если помпа на 2-х минутной паузе, то...
      MODE = 5;
      lcd.createChar(1, Buk_F); //
      lcd.createChar(2, Buk_I); //
      lcd.createChar(3, Buk_L); //
      lcd.createChar(4, Buk_MZ);
      lcd.createChar(5, Buk_C);
      lcd.createChar(6, Buk_JA);
      display_lcd(0, 0, stgName[stage]);
      display_lcd(6, 0, "  ");
      if (second == 0) {
        stageTime2 = stageTime;
        pumpTime2 = pumpTime;
      }
      else {
        stageTime2 = stageTime - 1;
        pumpTime2 = pumpTime + 1;
      }
      if (stageTime2 < 10) lcd.print("  "); //display_lcd(15,0,"0");
      else if (stageTime2 < 99)  lcd.print(" ");
      lcd.print("BPEM");
      lcd.write((uint8_t)6);
      lcd.print("=");
      lcd.print(stageTime2);
      display_lcd(17, 0, ":");
      if (second != 0) {
        if (60 - second < 10) display_lcd(18, 0, "0");
        lcd.print(60 - second);
      }
      else display_lcd(18, 0, "00");
      display_lcd(0, 1, "   "); // Фильтрация
      lcd.write((uint8_t)1);
      lcd.write((uint8_t)2);
      lcd.write((uint8_t)3);
      lcd.write((uint8_t)4);
      lcd.print("TPA");
      lcd.write((uint8_t)5);
      lcd.write((uint8_t)2);
      lcd.write((uint8_t)6);
      lcd.print(" - ");
      lcd.print(12 - pumpTime2);
      display_lcd(17, 1, ":");
      if (second != 0) {
        if (60 - second < 10) display_lcd(18, 1, "0");
        lcd.print(60 - second);
      }
      else display_lcd(18, 1, "00");
      display_lcd(0, 2, "   Tek. t = ");
      lcd.print(Temp_c);
      lcd.write((uint8_t)0);
      lcd.print("  ");
      display_lcd(0, 3, "- Exit +            ");
    }
    else {
      lcd.createChar(1, Buk_JA); //
      lcd.createChar(2, Buk_P); //
      lcd.createChar(3, Buk_z); //
      lcd.createChar(5, Buk_Y);
      lcd.createChar(6, Buk_t);
      display_lcd(0, 0, stgName[stage]);
      display_lcd(7, 0, " ");
      if (second == 0) stageTime2 = stageTime;
      else stageTime2 = stageTime - 1;
      if (stageTime2 < 10) lcd.print("  "); //display_lcd(15,0,"0");
      else if (stageTime2 < 99)  lcd.print(" ");
      lcd.print("BPEM");
      lcd.write((uint8_t)1);
      lcd.print("=");
      lcd.print(stageTime2);
      display_lcd(17, 0, ":");
      if (second != 0) {
        if (60 - second < 10) display_lcd(18, 0, "0");
        lcd.print(60 - second);
      }
      else display_lcd(18, 0, "00");
      lcd.setCursor(0, 1);
      lcd.print("   ");
      lcd.write((uint8_t)5);
      lcd.print("c");
      lcd.write((uint8_t)6);
      lcd.print(". t = ");
      lcd.print(stageTemp);
      lcd.write((uint8_t)0);
      if (stageTemp < 100) lcd.print("  ");
      else lcd.print(" ");
      display_lcd(0, 2, "   Tek. t = ");
      lcd.print(Temp_c);
      lcd.write((uint8_t)0);
      if (Temp_c < 100) lcd.print("  ");
      else lcd.print(" ");
      display_lcd(0, 3, "- Exit +  ");
      lcd.write((uint8_t)2);
      lcd.print("ay");
      lcd.write((uint8_t)3);
      lcd.print("a");
      pause_stage();             //    Проверить на ПАУЗУ
      change_temp(stageTemp, H_temp, L_temp);
      MODE = 6;
    }
    quit_mode (autoEnter);
    HTTP_loop();
    if (mainMenu == 0) autoEnter = false;
    delay(100);
  }
}






//  *****   Вывод надписи   Засыпьте солод
void add_malt (void)
{
  lcd.createChar(1, Buk_Z); //
  lcd.createChar(2, Buk_III); //
  lcd.createChar(3, Buk_P); //
  lcd.createChar(4, Buk_MZ);
  lcd.createChar(6, Buk_L);
  lcd.createChar(7, Buk_D);
  lcd.clear();
  digitalWrite(PUMP, HIGH);
  digitalWrite(HEAT, HIGH);
  display_lcd(0, 1, "   "); // Засыпь солода
  lcd.write((uint8_t)1);
  lcd.print("AC");
  lcd.write((uint8_t)2);
  lcd.write((uint8_t)3);
  lcd.write((uint8_t)4);
  lcd.print("TE CO");
  lcd.write((uint8_t)6);
  lcd.print("O");
  lcd.write((uint8_t)7);

  MODE = 3;
  HTTP_loop();
  Buzzer(3, 500);
  display_lcd(0, 3, "            Ok B"); // ОК  Выход
  lcd.write((uint8_t)2);
  lcd.print("XO");
  lcd.write((uint8_t)7);
  wait_for_confirm(malt);
  if (malt == false) {
    //            digitalWrite(HEAT,HIGH);
    //            digitalWrite(PUMP,HIGH);
    lcd.clear();
    //            delay(50);
    mainMenu = 0;
    autoEnter = false;
  }
}

//  *****   Вывод надписи   УДАЛИТЕ СОЛОД
void remove_malt (void)
{
  lcd.createChar(1, Buk_Y); //
  lcd.createChar(2, Buk_iii); //
  lcd.createChar(3, Buk_I); //
  lcd.createChar(4, Buk_L);
  lcd.createChar(5, Buk_D);
  lcd.createChar(6, Buk_III);
  lcd.createChar(7, Buk_d);
  lcd.clear();
  x = EC_Config.StageV;        // used add to stage count on the final stage for the resume
  EC_Config.StageN = x;        // Номер текущей Т-паузы
  EC_save();                   // записываем номер Т-паузы для восстановления
  digitalWrite(PUMP, HIGH);
  digitalWrite(HEAT, HIGH);
  MODE = 7;
  HTTP_loop();
  //  display_lcd(0,0,"  Remove  Malt  ");
  display_lcd(0, 1, "   "); //  УДАЛИТЕ СОЛОД
  lcd.write((uint8_t)1);
  lcd.write((uint8_t)5);
  lcd.print("A");
  lcd.write((uint8_t)4);
  lcd.write((uint8_t)3);
  lcd.print("TE  CO");
  lcd.write((uint8_t)4);
  lcd.print("O");
  lcd.write((uint8_t)5);

  Buzzer(3, 500);
  display_lcd(0, 3, "            Ok B"); // ОК  Выход
  lcd.write((uint8_t)2);
  lcd.print("xo");
  lcd.write((uint8_t)7);

  wait_for_confirm(malt);
  if (malt == false) {
    stageTime = EC_Config.BoilHopTim[0];
    EC_Config.StageT = stageTime; // saves the intial stage time
    EC_save();                    // Запись установок
    //          digitalWrite(HEAT,LOW);
    //          digitalWrite(PUMP,LOW);
    lcd.clear();
    //          delay(50);
    mainMenu = 0;
    autoEnter = false;
  }
}
// Загрузка температуры и времени текущей паузы
void get_stage_settings (void)
{
  stageTemp = EC_Config.StageTmp[EC_Config.StageN][EC_Config.Kpid[4]];
  stageTemp = stageTemp / 100;
  if (resume) {                // on the start of resume gets saved time
    stageTime = EC_Config.StageT;
    resume = false;            // clears resume for next loop
  }
  else {
    stageTime = EC_Config.StageTim[EC_Config.StageN][EC_Config.Kpid[4]]; // gets stage time
    EC_Config.StageT = stageTime; // saves the intial stage time
    EC_save();                    // Запись установок
  }

}

//  *************   Вывод надписи ВНЕСИТЕ ХМЕЛЬ
void hop_add (void)
{
  if (hopAdd < nmbrHops) {
    if (stageTime == hopTime) {
      MODE = 8;
      HTTP_loop();
      Buzzer(3, 500);
      lcd.createChar(1, Buk_I); //
      lcd.createChar(2, Buk_L); //
      lcd.createChar(3, Buk_MZ); //
      lcd.clear();
      display_lcd(0, 1, "   BHEC");   //  ВНЕСИТЕ ХМЕЛЬ
      lcd.write((uint8_t)1);
      lcd.print("TE  XME");
      lcd.write((uint8_t)2);
      lcd.write((uint8_t)3);
      delay(500);
      Buzzer(3, 500);
      hopAdd++;
      EC_Config.HopN  = hopAdd;
      EC_save();                    // Запись установок
      blhpAddr++;
      hopTime = EC_Config.BoilHopTim[blhpAddr];
      lcd.clear();
    }
  }
}

//  ************   Загрузка данных по хмелю
void get_boil_settings (void)
{
  nmbrHops = EC_Config.HopV - 1;
  if (resume) {
    if (x != EC_Config.StageV) {
      stageTime = EC_Config.StageTim[9][EC_Config.Kpid[4]];
    }
    else {
      stageTime = EC_Config.StageT;
    }
  }
  else {
    stageTime = EC_Config.StageTim[9][EC_Config.Kpid[4]];
    EC_Config.StageT = stageTime; // saves the intial stage time
    EC_save();                    // Запись установок
  }
  hopAdd = EC_Config.HopN;
  blhpAddr = hopAdd + 1;
  lcd.clear();
  hopTime = EC_Config.BoilHopTim[blhpAddr];
}

void auto_mode (void)
{
  nmbrStgs = EC_Config.StageV;// Загрузка начальных данных 1 паузы
  nmbrHops = EC_Config.HopV;//read the number of hop additions

  load_pid_settings();
  check_for_resume();             // Проверка на продолжение варки
  if (!(resume)) {                // Если варка сначала, то ...
    prompt_for_water();           // ВОДА НАЛИТА?
    wait_for_confirm(autoEnter);  // ждем подтверждения налитой воды
    if (!(autoEnter))return;      //  выход если воды нет
    pump_prime();                 //  Прокачка помпы
    EC_Config.StageN = 0;         // Номер текущей Т-паузы
    EC_save();                    // Запись установок
    x = 0;
  }
  if (autoEnter) {                    // mash steps
    EC_Config.Autobrew = 1;       // Режим АВТО запущен...      auto mode started
    EC_save();                    // Запись установок
    for (int i = x; i < nmbrStgs; i++) {
      EC_Config.StageN = x;        // Номер текущей Т-паузы
      EC_save();                   // записываем номер Т-паузы для восстановления
      x++;                         // used to count the stages for the resume
      tempReached = false;         // температура достигнута = false
      get_stage_settings();        // Загрузка температуры и времени текущей паузы
      start_time();
      stage_loop(i);
      if (!(autoEnter)) break;
      if (i == (nmbrStgs - 1) && autoEnter) { // at the end of the last step pauses to remove the malt pipe before the boil
        remove_malt();
        if (!(autoEnter))break;
      }
      Buzzer(1, 200);
      lcd.clear();
    }
  }
  // Старт кипячения
  if (autoEnter) {
    start_time();
    stageTemp = EC_Config.BoilTmp / 100; // загружаем температуру кипячения
    tempReached = false;
    get_boil_settings();
    stage_loop(EC_Config.StageV, 120, 80);
    if (autoEnter) {  // finishes the brewing process
      MODE = 9;
      HTTP_loop();
      lcd.createChar(1, Buk_Z); //  Вывод ВАРКА ЗАВЕРШЕНА
      lcd.createChar(2, Buk_SH);
      lcd.clear();
      display_lcd(0, 1, "  BAPKA "); //  ВАРКА ЗАВЕРШЕНА !!!
      lcd.write((uint8_t)1);
      lcd.print("ABEP");
      lcd.write((uint8_t)2);
      lcd.print("EHA !");

      Buzzer(5, 500);
      delay(1000);
      Buzzer(5, 500);
      delay(1000);
      Buzzer(5, 500);
      delay(2000);
      EC_Config.Autobrew = 0; // sets auto start byte to 0 for resume
      EC_Config.HopN = 0; // sets hop count to 0
      EC_save();                   // записываем номер Т-паузы для восстановления
      mainMenu = 0;
      autoEnter = false;
      resume = false;
    }
  }
}
// Gets the date and time from the ds1307 and prints result
void getDateDs1307() {
  // Reset the register pointer
  Wire.beginTransmission(clockAddress);
  Wire.write(byte(0x00));
  Wire.endTransmission();

  Wire.requestFrom(clockAddress, 7);

  // A few of these need masks because certain bits are control bits
  second     = bcdToDec(Wire.read() & 0x7f);
  minute     = bcdToDec(Wire.read());

  // Need to change this if 12 hour am/pm
  hour       = bcdToDec(Wire.read() & 0x3f);
  dayOfWeek  = bcdToDec(Wire.read());
  dayOfMonth = bcdToDec(Wire.read());
  day = dayOfMonth;
  month      = bcdToDec(Wire.read());
  year       = bcdToDec(Wire.read());
}

void setDateDs1307()
{
  // Use of (byte) type casting and ascii math to achieve result.
  /*second = (byte) ((Serial.read() - 48) * 10 + (Serial.read() - 48));
    minute = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
    hour  = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
    dayOfWeek = (byte) (Serial.read() - 48);
    dayOfMonth = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
    month = (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));
    year= (byte) ((Serial.read() - 48) *10 +  (Serial.read() - 48));*/

  Wire.beginTransmission(clockAddress);
  Wire.write(byte(0x00));
  second = 0; //Секунды
  Wire.write(decToBcd(second));  // 0 to bit 7 starts the clock
  minute = 0; // Минуты
  Wire.write(decToBcd(minute));
  hour = 0; // Часы
  Wire.write(decToBcd(hour));    // If you want 12 hour am/pm you need to set
  // bit 6 (also need to change readDateDs1307)
  dayOfWeek = 3; // День недели
  Wire.write(decToBcd(dayOfWeek));
  dayOfMonth = 20; // Число
  Wire.write(decToBcd(dayOfMonth));
  month = 4; // Месяц
  Wire.write(decToBcd(month));
  year = 16; // Год
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}
String getTimeStr() {
  String str = String(day) + "." + formatDigit(month, 2) + "." + formatDigit(year, 2) + " " + formatDigit(hour, 2) + ":" + formatDigit(minute, 2) + ":" + formatDigit(second, 2);
  return str;
}
String formatDigit(int i, int len) {
  String s = String(i);
  while (s.length() < len) {
    s = "0" + s;
  }
  return (s);
}
void setup() {
  // Последовательный порт для отладки
  Serial.begin(115200);
  //   Serial.print("\n\nFree memory "); Serial.print (ESP.getFreeHeap());
  // Инициализация дисплея
  lcd.init();
  lcd.backlight();   /* Make sure the backlight is turned on */
  lcd.clear();
  lcd.createChar(0, degc);
  // Инициируем кнопки
  pcf.write8(255);
  // Инициируем выходы управления
  pinMode (HEAT, OUTPUT);
  digitalWrite(HEAT, HIGH); // 1 - выключено
  pinMode (PUMP, OUTPUT);
  digitalWrite(PUMP, HIGH); // 1 - выключено
  pinMode (BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW); // 0 - выключено
  Buzzer(1, 500);                          // 1 короткий писк
  setDateDs1307();
  windowStartTime = millis();   //tell the PID to range between 0 and the full window size
  myPID.SetMode(AUTOMATIC);

  // Инициализация EEPROM
  EC_begin();
  EC_read();
  // Подключаемся к WiFi
  WiFi_begin();
  delay(1000);
  // Старт внутреннего WEB-сервера
  HTTP_begin();
}



void loop() {
  boolean quit;
  switch (mainMenu) {      //  Первый раз mainMenu=0
    // РУЧНОЙ РЕЖИМ
    case (1):
      lcd.createChar(1, Buk_CH); //  Вывод РУЧНОЙ РЕЖИМ
      lcd.createChar(2, Buk_II);
      lcd.createChar(3, Buk_J);
      lcd.createChar(4, Buk_I);
      lcd.createChar(5, Buk_Y);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("    P");
      lcd.write((uint8_t)5);
      lcd.write((uint8_t)1);
      lcd.print("HO");
      lcd.write((uint8_t)2);
      lcd.print(" PE");
      lcd.write((uint8_t)3);
      lcd.write((uint8_t)4);
      lcd.print("M    ");
      Buzzer(1, 500);
      delay (500);
      lcd.clear();
      manual_mode();
      mainMenu = 0;

      break;
    // РЕЖИМ АВТОМАТ
    case (2):
      lcd.createChar(1, Buk_J); //  Вывод РЕЖИМ АВТОМАТ
      lcd.createChar(2, Buk_I);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   PE");
      lcd.write((uint8_t)1);
      lcd.write((uint8_t)2);
      lcd.print("M ABTOMAT    ");
      Buzzer(1, 500);
      delay (500);
      lcd.clear();
      auto_mode();
      mainMenu = 0;
      MODE = 0;

      break;
    // НАСТРОЙКИ
    case (3):
      lcd.clear();
      lcd.createChar(1, Buk_II); //  Вывод НАСТРОЙКИ
      lcd.createChar(2, Buk_I);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("     HACTPO");
      lcd.write((uint8_t)1);
      lcd.print("K");
      lcd.write((uint8_t)2);
      Buzzer(1, 500);
      delay (500);
      lcd.clear();
      default_set();
      setup_mode();
      mainMenu = 0;
      break;
    // About
    case (4):
      lcd.clear();
      display_lcd(0, 0, "  Wi-Fi Homebrewer");
      display_lcd(0, 1, "  Firmware ver 1.0");
      if ( EC_Config.IP != 0 ) {
        display_lcd(0, 2, "  IP: ");
        lcd.print(WiFi.localIP());
      }
      else  display_lcd(0, 2, "  IP: 192.168.4.1");
      Buzzer(1, 500);
      for (uint8_t i = 0; i < 10; i++) {
        HTTP_loop();
        delay(500);
        if (mainMenu == 0) break;
      }
      mainMenu = 0;
      break;
    // Настройка параметров сети с компа
    case (5):
      lcd.clear();
      display_lcd(0, 1, "   Wi-Fi Control");
      display_lcd(0, 3, "- Exit +            ");
      quit = true;
      while (mainMenu == 5) {
        HTTP_loop();
        delay (100);
        quit_mode(quit);
        if (!quit) {
          mainMenu = 0;
        }
      }
      break;

    // Настройка параметров автоматики с компа
    case (6):
      lcd.clear();
      display_lcd(0, 1, "   Wi-Fi Control");
      display_lcd(0, 3, "- Exit +            ");
      quit = true;
      while (mainMenu == 6) {
        HTTP_loop();
        delay (100);
        quit_mode(quit);
        if (!quit) {
          mainMenu = 0;
        }
      }
      break;
    // Если ничего не нажато
    default:
      digitalWrite(HEAT, HIGH);  // Выключить ТЭН
      digitalWrite(PUMP, HIGH);  // Выключить насос
      Temperature();             // Считать температуру

      //  Вывод надписи ДОМАШНЯЯ ПИВОВАРНЯ
      lcd.createChar(1, Buk_D);
      lcd.createChar(2, Buk_SH);
      lcd.createChar(3, Buk_JA);
      lcd.createChar(4, Buk_P);
      lcd.createChar(5, Buk_I);
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.write((uint8_t)1);
      lcd.print("OMA");
      lcd.write((uint8_t)2);
      lcd.print("H");
      lcd.write((uint8_t)3);
      lcd.write((uint8_t)3);
      lcd.print(" ");
      lcd.write((uint8_t)4);
      lcd.write((uint8_t)5);
      lcd.print("BOBAPH");
      lcd.write((uint8_t)3);

      //  Вывод надписи Температура
      lcd.createChar(6, Buk_p);
      lcd.createChar(7, Buk_t);
      lcd.setCursor(0, 1);
      lcd.print(" Tem");
      lcd.write((uint8_t)6);
      lcd.print("epa");
      lcd.write((uint8_t)7);
      lcd.print("ypa: ");
      lcd.print(Temp_c);
      lcd.write((uint8_t)0);
      lcd.print(" ");
      //  вывод подписей к кнопкам
      display_lcd(0, 2, "About      Auto     ");
      display_lcd(0, 3, "     Manual    SetUp");



      //  Вот тут определяется выбор меню
      if (Button_1sec_press(Button_dn))mainMenu = 4;  //About MODE = 4;
      if (Button_1sec_press(Button_up))mainMenu = 1;  //Manual
      if (Button_1sec_press(Button_prev))mainMenu = 2;//Auto
      if (Button_1sec_press(Button_nxt))mainMenu = 3; //SetUp



      HTTP_loop();
      delay(100);

      //       getDateDs1307();
      //      String s = getTimeStr();  Serial.println(s);


      break;
  }

}




