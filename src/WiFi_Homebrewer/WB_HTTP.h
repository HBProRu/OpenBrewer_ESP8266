#ifndef WB_HTTP_h
#define WB_HTTP_h
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LiquidCrystal_I2C.h>

extern ESP8266WebServer server;
extern boolean isAP;
extern float Temp_c;
extern float mset_temp;
extern uint8_t MODE;
extern uint8_t mainMenu;
extern boolean mpump;  // Флаг включения насоса
extern boolean mheat;  // Флаг включения ТЭНа
extern uint32_t windowStartTime;
extern uint8_t HEAT;   // ТЭН
extern uint8_t PUMP;    // Насос
extern boolean resume;
extern boolean wtBtn;
extern boolean autoEnter;
extern boolean malt;
extern boolean stage_pause;
extern uint8_t  stageTime;
extern float stageTemp;


extern void display_lcd (uint8_t pos , uint8_t line , const char* lable);

boolean ConnectWiFi(void);
void HTTP_begin(void);
void HTTP_handleRoot(void);
void HTTP_handleReturnmain(void);
void HTTP_handleConfig(void);
void HTTP_handleToConfig(void);
void HTTP_handleConfig2(void);
void HTTP_handleToConfig2(void);
void HTTP_handleSave(void);
void HTTP_handleDefault(void);
void HTTP_handleReboot(void);
void HTTP_handleAbout(void);
void HTTP_handleManual(void);
void HTTP_handleToManual(void);
void HTTP_handleSaveManual(void);
void HTTP_handleAuto(void);
void HTTP_handleToAuto(void);
void HTTP_handleSaveAuto(void);
void HTTP_handleContinue(void);
void HTTP_handleSave2(void);
void HTTP_handlePause(void);
void HTTP_loop();
void WiFi_begin(void);


#endif
