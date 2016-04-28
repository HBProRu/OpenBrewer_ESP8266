#include "WB_HTTP.h"
#include "WB_EEPROM.h"

ESP8266WebServer server(80);
boolean isAP = false;
String authPass = "";
boolean enterry = false;
int Temp;
/**
   Старт WiFi
*/
void WiFi_begin(void) {
  // Подключаемся к WiFi
  display_lcd(0, 0, "Connect to Wi-Fi    ");
  isAP = false;
  if ( ! ConnectWiFi()  ) {
    Serial.printf("Start AP %s\n", EC_Config.ESP_NAME);
    display_lcd(0, 1, "Start AP,  Open     ");
    WiFi.mode(WIFI_STA);
    WiFi.softAP(EC_Config.ESP_NAME);
    isAP = true;
    Serial.print("Open http://192.168.4.1 in your browser\n");
    display_lcd(0, 2, " http://192.168.4.1 ");
    display_lcd(0, 3, " in your browser    ");
    delay (1000);
  }
  else {
    if ( EC_Config.StaticIP ) {
      // Получаем статический IP если нужно
      if ( EC_Config.IP != 0 ) {
        WiFi.config(EC_Config.IP, EC_Config.GW, EC_Config.MASK);
        Serial.print("Open http://");
        Serial.print(WiFi.localIP());
        Serial.println(" in your browser");
      }
    }
  }
  // Запускаем MDNS
  MDNS.begin(EC_Config.ESP_NAME);
  Serial.printf("Or by name: http://%s.local\n", EC_Config.ESP_NAME);
}

/**
   Соединение с WiFi
*/
boolean ConnectWiFi(void) {

  // Если WiFi не сконфигурирован
  if ( strcmp(EC_Config.AP_SSID, "none") == 0 ) {
    Serial.printf("Wi-Fi is not config ...\n");
    display_lcd(0, 1, "Wi-Fi is not config.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  // Пытаемся соединиться с точкой доступа
  Serial.printf("\nConnecting to: %s/%s\n", EC_Config.AP_SSID, EC_Config.AP_PASS);
  WiFi.begin(EC_Config.AP_SSID, EC_Config.AP_PASS);
  delay(1000);

  // Максиммум N раз проверка соединения (12 секунд)
  for ( int j = 0; j < 15; j++ ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nWi-Fi connect: ");
      Serial.print(WiFi.localIP());
      Serial.print("/");
      Serial.print(WiFi.subnetMask());
      Serial.print("/");
      Serial.println(WiFi.gatewayIP());
      display_lcd(0, 1, "Wi-Fi connect:  OK  ");
      //uint32_t S=WiFi.localIP();
      //      display_lcd(0,3,(const char*)&S);
      //      Serial.print(S);

      return true;
    }
    delay(1000);
    Serial.print(".");
    //    Serial.print(WiFi.status());
  }
  Serial.printf("\nConnect WiFi failed ...\n");
  display_lcd(0, 1, "WiFi connect: failed");
  delay(2000);
  return false;
}

/**
   Старт WEB сервера
*/
void HTTP_begin(void) {
  // Поднимаем WEB-сервер
  server.on ( "/", HTTP_handleRoot );
  server.on ( "/returnmain", HTTP_handleReturnmain );
  server.on ( "/config", HTTP_handleConfig );
  server.on ( "/toconfig", HTTP_handleToConfig );
  server.on ( "/config2", HTTP_handleConfig2 );
  server.on ( "/toconfig2", HTTP_handleToConfig2 );
  server.on ( "/save", HTTP_handleSave );
  server.on ( "/reboot", HTTP_handleReboot );
  server.on ( "/default", HTTP_handleDefault );
  server.on ( "/about", HTTP_handleAbout );
  server.on ( "/manual", HTTP_handleManual );
  server.on ( "/tomanual", HTTP_handleToManual );
  server.on ( "/savemanual", HTTP_handleSaveManual );
  server.on ( "/auto", HTTP_handleAuto );
  server.on ( "/toauto", HTTP_handleToAuto );
  server.on ( "/saveauto", HTTP_handleSaveAuto );
  server.on ( "/continue", HTTP_handleContinue );
  server.on ( "/save2", HTTP_handleSave2 );
  server.on ( "/pause", HTTP_handlePause );
  server.onNotFound ( HTTP_handleRoot );
  server.begin();
  Serial.print( "HTTP server started ...\n" );
  display_lcd(0, 2, "HTTP server started ");
  delay(2000);

}

// **************************************************************************************************
// * Обработчик событий WEB-сервера
// **************************************************************************************************
void HTTP_loop(void) {
  server.handleClient();
}


// **************************************************************************************************
// * Оработчик выхода на главную страницу
// **************************************************************************************************
void HTTP_handleReturnmain(void) {
  mainMenu = 0;
  //  enterry = false;
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик главной страницы сервера
// **************************************************************************************************
void HTTP_handleRoot(void) {
  String out = "";// Serial.print("\nstage_pause = "); Serial.print(stage_pause);
  if (stage_pause) {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/pause'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 1) {// Ручной режим
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/manual'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 2) {// Автоматический режим
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/auto'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 3) {// Режим настройки вручную (с кнопок блока)
    out =
      "<html>\
                        <head>\
                            <meta charset='utf-8' />\
                            <meta http-equiv='refresh' content='5'/>\
                            <title>Домашняя пивоварня - Ручная настройка</title>\
                              <style>\
                                     body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                              </style>\
                        </head>\
                          <body>\
                                <BR<BR><BR>><h1><center>Режим ручного управления</center></h1>\
                          </body>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 4) {// Справка об устройстве
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/about'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 5) {// Справка об устройстве
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/config'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 6) {// Справка об устройстве
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/config2'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  mainMenu = 0;
  out =
    "<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <meta http-equiv='refresh' content='5'/>\
    <title>Домашняя пивоварня - Главная</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
   <h1><center>WiFi управление домашней пивоварней</center></h1>\n";
  out += "<P><B><font size='4'><font color='black'> Имя в сети:</font><font color='red'><I> ";
  out += EC_Config.ESP_NAME;
  out += ".</I></font>";

  if ( isAP ) {
    out += "<font color='black'> Режим точки доступа:</font><font color='red'><I> ";
    out += EC_Config.ESP_NAME;
    out += ".</I></font></font></B></P>\n";
  }
  else {
    out += "<font color='black'> Подключено к:</font><font color='red'><I> ";
    out += EC_Config.AP_SSID;
    out += ".</I></font></font></B></P>\n";
  }
  char str[50];
  /*
     Здесь выводим что-нибудь свое

  */
  out += "<h2><font color='black'>";

  Temp = Temp_c;
  sprintf(str, "Текущая температура: </font>%d", Temp);
  out += str;
  //   int Temp2=Temp_c;
  Temp = (Temp_c - Temp) * 100;
  sprintf(str, ".%d °C</br>", Temp);
  out += str;
  out += "</h2>\n";
  out += "<h1><a href='/about'>Справка</a></h1>";
  out += "<h1><a href='/tomanual'>РУЧНОЙ РЕЖИМ</a></h1>";
  out += "<h1><a href='/toauto'>АВТОМАТИЧЕСКИЙ РЕЖИМ</a></h1>";
  out += "<h1><a href='/toconfig2'>Настройка контроллера автоматики</a></h1>";
  out += "<h1><a href='/toconfig'>Настройка параметров сети</a></h1>\
  </body>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик выхода на страницу Ручного режима
// **************************************************************************************************
void HTTP_handleToManual(void) {
  mainMenu = 1;
  //  enterry = false;
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/manual'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Обработчик ручного режима
// **************************************************************************************************
void HTTP_handleManual(void) {
  String out = "";
  if (stage_pause) {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/pause'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 0) {//  enterry = false;
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/returnmain'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 2 || mainMenu == 3 || mainMenu == 4 || mainMenu == 5 || mainMenu == 6) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  out =
    "<html>\
  <head>\
       <meta charset=\"utf-8\" />\
       <meta http-equiv='refresh' content='30' />\
       <title>Домашняя пивоварня - Ручной режим управления</title>\
       <style>\
             body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
       </style>\
  </head>\
  <body>\
       <h1><center>Ручной режим управления</center></h1>\n";
  out += "<P><B><font size='5'><font color='black'> Имя в сети:</font><font color='red'><I> ";
  out += EC_Config.ESP_NAME;
  out += ".</I></font>";

  if ( isAP ) {
    out += "<font color='black'> Режим точки доступа:</font><font color='red'><I>";
    out += EC_Config.ESP_NAME;
    out += ".</I></font></font></B></P><BR>";
  }
  else  {
    out += "<font color='black'> Подключено к:</font><font color='red'><I> ";
    out += EC_Config.AP_SSID;
    out += ".</I></font></font></B></P><BR>";
  }
  char str[100];
  Temp = Temp_c;                                                               //  Выводим текущую температуру
  out += "<form method='get' action='/savemanual'>\
              <table border=0>\
                   <tr><td><B><font size='6'><font color='black'>Текущая температура:</font></font><B></td>\
                       <td><B><font size='6'>";
  out += Temp;
  Temp = (Temp_c - Temp) * 100;
  sprintf(str, ".%d °C</font><B>", Temp);
  out += str;
  out += "</td>\
                   </tr>";

  Temp = mset_temp;                                                           //  Выводим установленную температуру
  out += "<tr><td><B><font size='6'><font color='black'>Установленная температура:</font></font><B></td>";
  out += "<td><input type='number' name='msettemp' style='font-size: 40px;'  size='2' min='20' max='120' step='0.25' value='";
  out += mset_temp;
  out += "'></td>"; // Окно для ввода новой температуры
  out += "<td><input type='submit' name='submit_temp' tabindex='0' style='font-size: 40px; border-radius: 5px;' value='Сохранить'></td>"; // Кнопка Сохранить
  out += "</tr>\
              </table><BR>";

  out += "<table border=0 align='center'><tr>\
                                      <td><button name='heat' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #";
  if (mheat == false) {
    out += "00FF00;'><font color='black'>Включить ТЭН";
  }
  else {
    out += "FF8080;'><font color='black'>Выключить ТЭН";
  }
  out += "</font></button></td>";         // Кнопка ТЭН
  out += "<td><P>   </P></td>";
  out += "<td><button name='pump' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #";
  if (mpump == false) {
    out += "00FF00;'><font color='black'>Включить НАСОС";
  }
  else {
    out += "FF8080;'><font color='black'>Выключить НАСОС";
  }
  out += "</font></button></td>";         // Кнопка НАСОС
  out += "</tr>\
              </table>\
      </form>\
      <BR><P align='center'><B><font size='10'><a href='/returnmain'>Выход</a></font><B></P>\
  </body>\
</html>";
  //   size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Обработчик изменения ручного управления
// **************************************************************************************************
void HTTP_handleSaveManual(void) {
  if ( server.hasArg("msettemp")     ) {
    mset_temp = atof(server.arg("msettemp").c_str());
    Temp = mset_temp;
  }
  if (server.hasArg("heat"))          {
    if (mheat == false) {
      mheat = true;
      windowStartTime = millis();
    }
    else {
      mheat = false;
      digitalWrite(HEAT, HIGH);
    }
    // Serial.print("\nmheat = "); Serial.print(mheat);
  }
  if ( server.hasArg("pump")         ) {
    if (mpump == false) {
      mpump = true;
      digitalWrite(PUMP, LOW);
    }
    else {
      mpump = false;
      digitalWrite(PUMP, HIGH);
    }
    // Serial.print("\nmpump = "); Serial.print(mpump);
  }
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/manual'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик выхода на страницу Автоматического режима
// **************************************************************************************************
void HTTP_handleToAuto(void) {
  mainMenu = 2;
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/auto'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Обработчик автоматического режима
// **************************************************************************************************
void HTTP_handleAuto(void) {
  String out = "";
  // Serial.print("\nMODE = "); Serial.print(MODE);
  // Serial.print("\nmainMenu = "); Serial.print(mainMenu);
  if (stage_pause)  {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/pause'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 0) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/returnmain'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 1 || mainMenu == 3 || mainMenu == 4 || mainMenu == 5 || mainMenu == 6) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (MODE == 1 || MODE == 2 || MODE == 3 || MODE == 7) { //  Продолжить варку? Вода налита? Засыпьте солод.
    out = "<html>\
                 <head>\
                      <meta charset=\"utf-8\" />\
                      <meta http-equiv='refresh' content='5' />\
                      <title>Домашняя пивоварня - Автоматический режим управления</title>\
                      <style>\
                            body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                      </style>\
                 </head>\
                 <body>\
                      <h1><center>Автоматический режим управления</center></h1>\n";
    out += "<P><B><font size='5'><font color='black'> Имя в сети:</font><font color='red'><I> ";
    out += EC_Config.ESP_NAME;
    out += ".</I></font>";
    if ( isAP ) {
      out += "<font color='black'> Режим точки доступа:</font><I> <font color='red'>";
      out += EC_Config.ESP_NAME;
      out += ".</I></font></font></B></P>\n";
    }
    else  {
      out += "<font color='black'> Подключено к:</font><font color='red'><I> ";
      out += EC_Config.AP_SSID;
      out += ".</I></font> </font></B></P><BR><BR>";
    }
    if (MODE == 1) out += "<h1><center>Продолжить варку?</center></h1><BR><BR>";
    else if (MODE == 2) out += "<h1><center>Вода налита?</center></h1><BR><BR>";
    else if (MODE == 3) out += "<h1><center>Засыпьте солод в бункер</center></h1><BR><BR>";
    else if (MODE == 7) out += "<h1><center>Удалите солод из бункера</center></h1><BR><BR>";
    char str[100];
    out += "<form method='get' action='/continue'>";
    out += "<table border=0 align='center'><tr>";
    out += "<td><button name='yes' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #00FF00;'>\
                                      <font color='black'>Да</font></button></td>";          // Кнопка Да
    out += "<td><P>   </P></td>";
    out += "<td><button name='no' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #80FF00;'>\
                                      <font color='black'>Выход</font></button></td>";          // Кнопка Нет
    out += "</tr>\
                          </table>\
                       </form>";
    out += "</body>\
           </html>";
    //   size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
    server.send ( 200, "text/html", out );
    return;
  }

  if (MODE == 4 || MODE == 5 || MODE == 6 || MODE == 8) { //  Прогон по паузам?
    out =
      "<html>\
             <head>\
                  <meta charset=\"utf-8\" />\
                  <meta http-equiv='refresh' content='10'>\
                  <title>Домашняя пивоварня - Автоматический режим управления</title>\
                  <style>\
                        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                  </style>\
             </head>\
             <body>\
                  <h1><center>Автоматический режим управления</center></h1>\n";
    out += "<P><B><font size='5'><font color='black'> Имя в сети:</font><font color='red'><I> ";
    out += EC_Config.ESP_NAME;
    out += ".</I></font>";

    if ( isAP ) {
      out += "<font color='black'> Режим точки доступа:</font><I> <font color='red'>";
      out += EC_Config.ESP_NAME;
      out += ".</I></font></font></B></P>\n";
    }
    else  {
      out += "<font color='black'> Подключено к:</font><font color='red'><I> ";
      out += EC_Config.AP_SSID;
      out += ".</I></font> </font></B></P><BR>";
    }
    char str[100];
    Temp = Temp_c;                                                               //  Выводим текущую температуру
    out += "<form method='get' action='/saveauto'>\
                         <table border=0>";
    out += "<tr><td><B><font size='6'><font color='black'>Температурная пауза:</font></font><B></td>\
                                  <td><B><font size='6'>";
    if (EC_Config.StageN == 0) out += "MashIn";
    else if (EC_Config.StageN == (EC_Config.StageV - 1)) out += "MashOut";
    else if (EC_Config.StageN == EC_Config.StageV) out += "Кипячение";
    else out += EC_Config.StageN;
    out += "</font><B></td>\
                              </tr>\n";
    out += "<tr><td><B><font size='6'><font color='black'>Время паузы:</font></font><B></td>\
                                  <td><B><font size='6'>";
    out += stageTime;
    out += " мин.</font><B></td>\
                              </tr>\n";



    out += "<tr><td><B><font size='6'><font color='black'>Текущая температура:</font></font><B></td>\
                                  <td><B><font size='6'>";
    out += Temp;
    Temp = (Temp_c - Temp) * 100;
    sprintf(str, ".%d °C</font><B>", Temp);
    out += str;
    out += "</td>\
                              </tr>\n";

    Temp = mset_temp;                                                           //  Выводим установленную температуру pattern='\\d+(\\.\\d{2})?'
    out += "<tr><td><B><font size='6'><font color='black'>Установленная температура:</font></font><B></td>";
    out += "<td><input type='number' name='msettemp' style='font-size: 40px;'";
    if (EC_Config.StageN == EC_Config.StageV) {
      out += "min='80' max='120'";
    }
    else {
      out += "min='20' max='80'";
    }
    out += " step='0.25' value='";
    out += stageTemp;
    out += "'></td>";  // Окно для ввода новой температуры
    out += "<td><input type='submit' name='submit_temp' style='font-size: 40px; border-radius: 5px;' value='Сохранить'></td>"; // Кнопка Сохранить
    out += "</tr>\
                         </table><BR>";
    if (MODE == 4) out += "<BR><h1><center>Прокачка помпы</center></h1><BR>";
    else if (MODE == 5) out += "<BR><h1><center>Фильтрация</center></h1><BR>";
    else if (MODE == 8) out += "<BR><h1><center>Внесите хмель в сусло</center></h1><BR>";
    out += "<table border=0 align='center'>\
                                   <tr>\
                                      <td><button name='pause' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #00FF00;'>\
                                      <font color='black'>Пауза</font></button></td>";          // Кнопка Пауза
    out += "<td><P>   </P></td>";
    out += "<td><P align='center'><B><font size='14'><a href='/returnmain'>Выход</a></font><B></P></td>";
    out += "</tr>\
                         </table>\
      </form>\
  </body>\
</html>";
    //   size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
    server.send ( 200, "text/html", out );
  }
  if (MODE == 9) { //  ВАРКА ЗАВЕРШЕНА
    out = "<html>\
                 <head>\
                      <meta charset=\"utf-8\" />\
                      <meta http-equiv='refresh' content='5' />\
                      <title>Домашняя пивоварня - Автоматический режим управления</title>\
                      <style>\
                            body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                      </style>\
                 </head>\
                 <body>\
                      <P><B><font size='20'><font color='black'>ВАРКА ЗАВЕРШЕНА!!!</font></font></B></P>\
                 </body>\
           </html>";
    //   size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
    server.send ( 200, "text/html", out );
  }
}

// **************************************************************************************************
// * Оработчик Паузы
// **************************************************************************************************
void HTTP_handlePause(void) {
  String out = "";
  if (!stage_pause)  {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/auto'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  out =
    "<html>\
                 <head>\
                     <meta charset='utf-8' />\
                     <meta http-equiv='refresh' content='3'/>\
                     <title>Домашняя пивоварня - Пауза</title>\
                     <style>\
                         body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                     </style>\
                </head>\
                    <body>\
                       <BR<BR><BR><h1><center>ПАУЗА</center></h1><BR<BR><BR>\
                       <form method='get' action='/saveauto'>\
                           <table border=0 align='center'>\
                                 <tr>\
                                    <td><button name='pause' style=' width: 300px; height: 100px; border-radius: 20px; box-shadow: 0px 1px 3px; font-size: 40px; background: #00FF00;'>\
                                         <font color='black'>НАЗАД</font></button></td>\
                                 </tr>\
                           </table>\
                       </form>\
                    </body>\
            </html>";
  server.send ( 200, "text/html", out );
}
// **************************************************************************************************
// * Обработчик продолжения варки
// **************************************************************************************************
void HTTP_handleContinue(void) {
  if ( server.hasArg("yes")         ) {
    if (MODE == 1) {
      resume = true;
      wtBtn = false;
    }
    else if (MODE == 2) {
      autoEnter = true;
      wtBtn = false;
    }
    else if (MODE == 3) {
      malt = true;
      wtBtn = false;
    }
    else if (MODE == 7) {
      malt = true;
      wtBtn = false;
    }
  }
  if ( server.hasArg("no")         ) {
    if (MODE == 1) {
      resume = false;
      wtBtn = false;
    }
    else if (MODE == 2) {
      autoEnter = false;
      wtBtn = false;
    }
    else if (MODE == 3) {
      malt = false;
      wtBtn = false;
    }
    else if (MODE == 7) {
      malt = false;
      wtBtn = false;
    }
  }

  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/auto'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );

}

// **************************************************************************************************
// * Обработчик изменения автоматического управления
// **************************************************************************************************
void HTTP_handleSaveAuto(void) {
  if ( server.hasArg("msettemp")     ) {
    stageTemp = atof(server.arg("msettemp").c_str());
  }
  if ( server.hasArg("pause")        ) {
    if (stage_pause == false) {
      stage_pause = true;// Serial.print("\nstage_pause = "); Serial.print(stage_pause);

    }
    else {
      stage_pause = false;// Serial.print("\nstage_pause = "); Serial.print(stage_pause);
    }
    // Serial.print("\nmpump = "); Serial.print(mpump);
  }
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/auto'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик выхода на страницу настройки сервера
// **************************************************************************************************
void HTTP_handleToConfig(void) {
  mainMenu = 5;
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/config'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик страницы настройки сервера
// **************************************************************************************************
void HTTP_handleConfig(void) {
  String out = "";
  if (stage_pause) {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/pause'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 0) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/returnmain'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 1 || mainMenu == 2 || mainMenu == 3 || mainMenu == 4 || mainMenu == 6) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  char str[10];
  out =
    "<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <title>Домашняя пивоварня - Настройка параметров сети</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Настройка параметров сети</h1>\
     <ul>\
     <li><a href=\"/returnmain\">Главная</a>\
     <li><a href=\"/toconfig2\">Настройка параметров автоматики</a>\
     <li><a href=\"/reboot\">Перезагрузка</a>\
     <li><a href=\"/default\">Сброс настроек</a>\
     </ul>\n";

  out +=
    "     <form method='get' action='/save'>\
         <p><b>Параметры ESP модуля</b></p>\
         <table border=0><tr><td>Наименование:</td><td><input name='esp_name' length=32 value='";
  out += EC_Config.ESP_NAME;
  out += "'></td></tr>\
         <tr><td>Пароль admin:</td><td><input name='esp_pass' length=32 value='";
  out += EC_Config.ESP_PASS;
  out += "'></td></tr></table>\
         <p><b>Использовать статический IP адрес</b><input type='checkbox' name='staticip' value='static'";
  if (EC_Config.StaticIP) out += "checked";
  out += "></p>\
         <p><b>Параметры WiFi подключения</b></p>\
         <table border=0><tr><td>Имя сети: </td><td><input name='ap_ssid' length=32 value='";
  out += EC_Config.AP_SSID;
  out += "'></td></tr>\
         <tr><td>Пароль:</td><td><input name='ap_pass' length=32 value='";
  out += EC_Config.AP_PASS;
  out += "'></td></tr>\
        <tr><td>IP-адрес:</td><td><input name='ip1' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.IP[0]);
  out += str;
  out += "'>.<input name='ip2' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.IP[1]);
  out += str;
  out += "'>.<input name='ip3' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.IP[2]);
  out += str;
  out += "'>.<input name='ip4' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.IP[3]);
  out += str;
  out += "'></td></tr>\
        <tr><td>IP-маска:</td><td><input name='mask1' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.MASK[0]);
  out += str;
  out += "'>.<input name='mask2' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.MASK[1]);
  out += str;
  out += "'>.<input name='mask3' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.MASK[2]);
  out += str;
  out += "'>.<input name='mask4' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.MASK[3]);
  out += str;
  out += "'></td></tr>\
        <tr><td>IP-шлюз:</td><td><input name='gw1' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.GW[0]);
  out += str;
  out += "'>.<input name='gw2' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.GW[1]);
  out += str;
  out += "'>.<input name='gw3' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.GW[2]);
  out += str;
  out += "'>.<input name='gw4' length=4 size=4 value='";
  sprintf(str, "%d", EC_Config.GW[3]);
  out += str;
  out += "'></td></tr></table><BR>";
  out += "<input type='submit' style='font-size: 40px; border-radius: 5px;' value='Сохранить'>\
     </form>\
  </body>\
</html>";
  //   size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик сохранения сетевых настроек
// **************************************************************************************************
void HTTP_handleSave(void) {

  if ( server.hasArg("esp_name")     )strcpy(EC_Config.ESP_NAME, server.arg("esp_name").c_str());
  if ( server.hasArg("esp_pass")     )strcpy(EC_Config.ESP_PASS, server.arg("esp_pass").c_str());
  if ( server.hasArg("ap_ssid")      )strcpy(EC_Config.AP_SSID, server.arg("ap_ssid").c_str());
  if ( server.hasArg("ap_pass")      )strcpy(EC_Config.AP_PASS, server.arg("ap_pass").c_str());
  if ( server.hasArg("ip1")          )EC_Config.IP[0] = atoi(server.arg("ip1").c_str());
  if ( server.hasArg("ip2")          )EC_Config.IP[1] = atoi(server.arg("ip2").c_str());
  if ( server.hasArg("ip3")          )EC_Config.IP[2] = atoi(server.arg("ip3").c_str());
  if ( server.hasArg("ip4")          )EC_Config.IP[3] = atoi(server.arg("ip4").c_str());
  if ( server.hasArg("mask1")        )EC_Config.MASK[0] = atoi(server.arg("mask1").c_str());
  if ( server.hasArg("mask2")        )EC_Config.MASK[1] = atoi(server.arg("mask2").c_str());
  if ( server.hasArg("mask3")        )EC_Config.MASK[2] = atoi(server.arg("mask3").c_str());
  if ( server.hasArg("mask4")        )EC_Config.MASK[3] = atoi(server.arg("mask4").c_str());
  if ( server.hasArg("gw1")          )EC_Config.GW[0] = atoi(server.arg("gw1").c_str());
  if ( server.hasArg("gw2")          )EC_Config.GW[1] = atoi(server.arg("gw2").c_str());
  if ( server.hasArg("gw3")          )EC_Config.GW[2] = atoi(server.arg("gw3").c_str());
  if ( server.hasArg("gw4")          )EC_Config.GW[3] = atoi(server.arg("gw4").c_str());
  if ( server.hasArg("staticip")     )EC_Config.StaticIP  = true;
  else EC_Config.StaticIP  = false;
  EC_save();
  String out = "";
  out = "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/returnmain'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик выхода на страницу настройки контроллера автоматики
// **************************************************************************************************
void HTTP_handleToConfig2(void) {
  mainMenu = 6;
  String out = "";
  out =
    "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/config2'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );
}

// **************************************************************************************************
// * Оработчик страницы настройки контроллера автоматики
// **************************************************************************************************
void HTTP_handleConfig2(void) {
  String out = "";
  if (stage_pause) {// Пауза
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/pause'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 0) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/returnmain'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  if (mainMenu == 1 || mainMenu == 2 || mainMenu == 3 || mainMenu == 4 || mainMenu == 5) {
    out =
      "<html>\
                        <head>\
                           <meta http-equiv='refresh' content='0;URL=/'>\
                        </head>\
                     </html>";
    server.send ( 200, "text/html", out );
    return;
  }
  char str[5];
  out =
    "<html>\
  <head>\
    <meta charset=\"utf-8\" />\
    <title>Домашняя пивоварня - Настройка параметров автоматики</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1><center>Настройка параметров автоматики</h1>\
     <ul>\
     <li><a href=\"/returnmain\">Главная</a>\
     <li><a href=\"/toconfig\">Настройка параметров сети</a>\
     <li><a href=\"/reboot\">Перезагрузка</a>\
     <li><a href=\"/default\">Сброс настроек</a>\
     </ul>";

  out +=
    "     <form method='get' action='/save2'>\
         <p><b>Параметры работы ПИД - регулятора</b></p>\
         <table border=0><tr><td>Коэффициент пропорциональный Kp:</td><td><input type='number' name='Kp' min='-100' max='100' value='";
  sprintf(str, "%d", EC_Config.Kpid[0]);
  out += str;
  out += "'></td></tr>\
            <tr><td>Коэффициент интегральный Ki:</td><td><input type='number' name='Ki' min='-100' max='100' value='";
  sprintf(str, "%d", EC_Config.Kpid[1]);
  out += str;
  out += "'></td></tr>\
            <tr><td>Коэффициент дифференциальный Kd:</td><td><input type='number' name='Kd' min='-100' max='100' value='";
  sprintf(str, "%d", EC_Config.Kpid[2]);
  out += str;
  out += "'></td></tr>\
            <tr><td>Временное окно, мсек:</td><td><input type='number' name='wsize' min='500' max='5000' step='500' value='";
  sprintf(str, "%d", EC_Config.Kpid[3]);
  out += str;
  out += "'></td></tr>\
         </table><br>";
  out += "<table border=0><tr><th>Номер текущего рецепта:</th><td><input type='number' name='Receipt' min='0' max='8' value='";
  sprintf(str, "%d", EC_Config.Kpid[4]);
  out += str;
  out += "'></td>";
  //                out +="<td><input type='submit' name='minus_rec' style='font-size: 30px; width: 40px; height: 40px; border-radius: 5px;' value='-'></td>";          // Кнопка -
  //                out +="<td><input type='submit' name='plus_rec' style='font-size: 30px; width: 40px; height: 40px; border-radius: 5px;' value='+'></td>";           // Кнопка +
  out += "</tr>\
         </table><br>";

  out += "<table border=1>\
                        <tr align='center'><th colspan='3'>Температурные паузы</th></tr>\
                        <tr align='center'><th>Пауза</th><th>Температура, °С</th><th>Время, мин.</th></tr>\
                        <tr align='center'><th >MashIn</th><td><input name='Temp0' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[0][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td>\
                                  <td><input name='Tim0' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[0][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                        <tr align='center'><th>1</th><td><input name='Temp1' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[1][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim1' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[1][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                        <tr align='center'><th>2</th><td><input name='Temp2' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[2][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim2' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[2][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                        <tr align='center'><th>3</th><td><input name='Temp3' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[3][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim3' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[3][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                         <tr align='center'><th>4</th><td><input name='Temp4' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[4][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim4' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[4][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                         <tr align='center'><th>5</th><td><input name='Temp5' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[5][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim5' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[5][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                         <tr align='center'><th>6</th><td><input name='Temp6' type='number' min='20' max='80' value='";//
  sprintf(str, "%d", (EC_Config.StageTmp[6][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim6' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[6][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                         <tr align='center'><th>7</th><td><input name='Temp7' type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[7][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim7' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[7][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                         <tr align='center'><th>MashOut</th><td><input name='Temp8'type='number' min='20' max='80' value='";
  sprintf(str, "%d", (EC_Config.StageTmp[8][EC_Config.Kpid[4]] / 100));
  out += str;
  out += "'></td><td><input name='Tim8' type='number' min='0' max='90' value='";
  sprintf(str, "%d", EC_Config.StageTim[8][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr>\
                  </table><br>";
  out += "<table border=0><tr><th>Температура кипячения, °С:</th><td><input name='Tempboil' type='number' min='80' max='120' value='";
  sprintf(str, "%d", EC_Config.BoilTmp / 100);
  out += str;
  out += "'></td></tr>\
                                          <tr><th>Время кипячения, мин. :</th><td><input name='TimBoil' type='number' min='0' max='120' value='";
  sprintf(str, "%d", EC_Config.StageTim[9][EC_Config.Kpid[4]]);
  out += str;
  out += "'></td></tr></table><br>";
  out += "<table border=1>\
                                 <tr align='center'><th colspan='2'>Время вноса хмеля, мин. до конца варки</th></tr>\
                                 <tr align='center'><th>1</th><td><input name='TempHop1' type='number' min='0' max='120' value='";
  sprintf(str, "%d", (EC_Config.BoilHopTim[1]));
  out += str;
  out += "'></td></tr>\
                                 <tr align='center'><th>2</th><td><input name='TempHop2' type='number' min='0' max='120' value='";
  sprintf(str, "%d", (EC_Config.BoilHopTim[2]));
  out += str;
  out += "'></td></tr>\
                                 <tr align='center'><th>3</th><td><input name='TempHop3' type='number' min='0' max='120' value='";
  sprintf(str, "%d", (EC_Config.BoilHopTim[3]));
  out += str;
  out += "'></td></tr>\
                                 <tr align='center'><th>4</th><td><input name='TempHop4' type='number' min='0' max='120' value='";
  sprintf(str, "%d", (EC_Config.BoilHopTim[4]));
  out += str;
  out += "'></td></tr>\
                                 <tr align='center'><th>5</th><td><input name='TempHop5' type='number' min='0' max='120' value='";
  sprintf(str, "%d", (EC_Config.BoilHopTim[5]));
  out += str;
  out += "'></td></tr>\
                  </table><br>";

  out += "<table border=0 align='center'><tr><td><input type='submit' style='font-size: 40px; border-radius: 5px;' value='Сохранить'></td></tr></table>\
     </form>\
  </body>\
</html>";
  size_t sz1 = strlen(out.c_str());   Serial.print("\nSize out = "); Serial.print((int)sz1);
  server.send ( 200, "text/html", out );
}


// **************************************************************************************************
// * Оработчик сохранения настроек автоматики
// **************************************************************************************************
void HTTP_handleSave2(void) {
  if ( server.hasArg("Kp")     ) EC_Config.Kpid[0] = atoi(server.arg("Kp").c_str());
  if ( server.hasArg("Ki")     ) EC_Config.Kpid[1] = atoi(server.arg("Ki").c_str());
  if ( server.hasArg("Kd")     ) EC_Config.Kpid[2] = atoi(server.arg("Kd").c_str());
  if ( server.hasArg("wsize")  ) EC_Config.Kpid[3] = atoi(server.arg("wsize").c_str());
  if ( server.hasArg("Temp0")  ) EC_Config.StageTmp[0][EC_Config.Kpid[4]] = (atoi(server.arg("Temp0").c_str())) * 100;
  if ( server.hasArg("Temp1")  ) EC_Config.StageTmp[1][EC_Config.Kpid[4]] = (atoi(server.arg("Temp1").c_str())) * 100;
  if ( server.hasArg("Temp2")  ) EC_Config.StageTmp[2][EC_Config.Kpid[4]] = (atoi(server.arg("Temp2").c_str())) * 100;
  if ( server.hasArg("Temp3")  ) EC_Config.StageTmp[3][EC_Config.Kpid[4]] = (atoi(server.arg("Temp3").c_str())) * 100;
  if ( server.hasArg("Temp4")  ) EC_Config.StageTmp[4][EC_Config.Kpid[4]] = (atoi(server.arg("Temp4").c_str())) * 100;
  if ( server.hasArg("Temp5")  ) EC_Config.StageTmp[5][EC_Config.Kpid[4]] = (atoi(server.arg("Temp5").c_str())) * 100;
  if ( server.hasArg("Temp6")  ) EC_Config.StageTmp[6][EC_Config.Kpid[4]] = (atoi(server.arg("Temp6").c_str())) * 100;
  if ( server.hasArg("Temp7")  ) EC_Config.StageTmp[7][EC_Config.Kpid[4]] = (atoi(server.arg("Temp7").c_str())) * 100;
  if ( server.hasArg("Temp8")  ) EC_Config.StageTmp[8][EC_Config.Kpid[4]] = (atoi(server.arg("Temp8").c_str())) * 100;
  if ( server.hasArg("Tim0")   ) EC_Config.StageTim[0][EC_Config.Kpid[4]] = atoi(server.arg("Tim0").c_str());
  if ( server.hasArg("Tim1")   ) EC_Config.StageTim[1][EC_Config.Kpid[4]] = atoi(server.arg("Tim1").c_str());
  if ( server.hasArg("Tim2")   ) EC_Config.StageTim[2][EC_Config.Kpid[4]] = atoi(server.arg("Tim2").c_str());
  if ( server.hasArg("Tim3")   ) EC_Config.StageTim[3][EC_Config.Kpid[4]] = atoi(server.arg("Tim3").c_str());
  if ( server.hasArg("Tim4")   ) EC_Config.StageTim[4][EC_Config.Kpid[4]] = atoi(server.arg("Tim4").c_str());
  if ( server.hasArg("Tim5")   ) EC_Config.StageTim[5][EC_Config.Kpid[4]] = atoi(server.arg("Tim5").c_str());
  if ( server.hasArg("Tim6")   ) EC_Config.StageTim[6][EC_Config.Kpid[4]] = atoi(server.arg("Tim6").c_str());
  if ( server.hasArg("Tim7")   ) EC_Config.StageTim[7][EC_Config.Kpid[4]] = atoi(server.arg("Tim7").c_str());
  if ( server.hasArg("Tim8")   ) EC_Config.StageTim[8][EC_Config.Kpid[4]] = atoi(server.arg("Tim8").c_str());
  if ( server.hasArg("Tempboil")) EC_Config.BoilTmp = (atoi(server.arg("Tempboil").c_str())) * 100;
  if ( server.hasArg("TimBoil"))  EC_Config.StageTim[9][EC_Config.Kpid[4]] = atoi(server.arg("TimBoil").c_str());
  if ( server.hasArg("TempHop1")) {
    EC_Config.BoilHopTim[1] = atoi(server.arg("TempHop1").c_str());
    if ( EC_Config.BoilHopTim[1] > EC_Config.StageTim[9][EC_Config.Kpid[4]] ) EC_Config.BoilHopTim[1] =  EC_Config.StageTim[9][EC_Config.Kpid[4]];
  }
  if ( server.hasArg("TempHop2")) {
    EC_Config.BoilHopTim[2] = atoi(server.arg("TempHop2").c_str());
    if ( EC_Config.BoilHopTim[2] > EC_Config.BoilHopTim[1] ) EC_Config.BoilHopTim[2] = EC_Config.BoilHopTim[1];
  }
  if ( server.hasArg("TempHop3")) {
    EC_Config.BoilHopTim[3] = atoi(server.arg("TempHop3").c_str());
    if ( EC_Config.BoilHopTim[3] > EC_Config.BoilHopTim[2] ) EC_Config.BoilHopTim[3] = EC_Config.BoilHopTim[2];
  }
  if ( server.hasArg("TempHop4")) {
    EC_Config.BoilHopTim[4] = atoi(server.arg("TempHop4").c_str());
    if ( EC_Config.BoilHopTim[4] > EC_Config.BoilHopTim[3] ) EC_Config.BoilHopTim[4] = EC_Config.BoilHopTim[3];
  }
  if ( server.hasArg("TempHop5")) {
    EC_Config.BoilHopTim[5] = atoi(server.arg("TempHop5").c_str());
    if ( EC_Config.BoilHopTim[5] > EC_Config.BoilHopTim[4] ) EC_Config.BoilHopTim[5] = EC_Config.BoilHopTim[4];
  }
  if ( server.hasArg("Receipt")) EC_Config.Kpid[4] = atoi(server.arg("Receipt").c_str());

  EC_save();

  String out = "";
  out = "<html>\
  <head>\
    <meta http-equiv='refresh' content='0;URL=/config2'>\
  </head>\
</html>";
  server.send ( 200, "text/html", out );


}
/*
   Сброс настрое по умолчанию
*/
void HTTP_handleDefault(void) {
  EC_default();
  HTTP_handleConfig();
}

/*
   Перезагрузка контроллера
*/
void HTTP_handleReboot(void) {

  String out = "";

  out =
    "<html>\
  <head>\
    <meta charset='utf-8' />\
    <meta http-equiv='refresh' content='30;URL=/returnmain'>\
    <title>Домашняя пивоварня</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Перезагрузка контроллера</h1>\
     <p><a href=\"/returnmain\">Через 30 сек будет переадресация на главную</a>\
   </body>\
</html>";
  server.send ( 200, "text/html", out );
  ESP.reset();

}

/*
   Информация об оборудовании
*/
void HTTP_handleAbout(void) {
  String out = "";
  mainMenu = 4;
  out =
    "<html>\
  <head>\
    <meta charset='utf-8' />\
    <meta http-equiv='refresh' content='5;URL=/returnmain'>\
    <title>Домашняя пивоварня - Информация об оборудовании</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Информация об оборудовании</h1>\
     <p>Домашняя пивоварня с управлением по Wi-Fi\
     <p><a href=\"/returnmain\">Главная -- Через 5 сек будет переадресация на главную</a>\
   </body>\
</html>";
  server.send ( 200, "text/html", out );

}



