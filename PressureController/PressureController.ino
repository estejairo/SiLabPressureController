/*
 * 
 * hola
 *
*/

/*////// INCLUDES \\\\\\*/
//Arduino serial communication
#include <Wire.h>
#include "SPI.h"

/*//TFT Screen*/
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"

//Diferential Pressure Sensors*/
#include "SDP.h"

/*////// MACROS \\\\\\*/
#define TCAADDR 0x70 //I2C multiplexor base address
#define TFT_DC 9  //Touchscreen data/command selector
#define TFT_CS 10 //Touchscreen chip selector
// FSM States
#define STAND_BY  0
#define INIT      1
#define MEASURE   2
#define RESULTS   3
#define SEND      4
#define SETTINGS  5
#define SHUT_DOWN 6
#define ALARM     7
//Pinout
#define BUZZER_PIN      2
#define KILL_PIN        3
#define INTERRUPT_PIN   5
#define LOWBATTERY_PIN  6

/*////// GLOBALS \\\\\\*/
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();

int current_touch = 0;
int last_touch = 0;
TS_Point current_point;
TS_Point last_point;

SDP_Controller SDP800;

float DP;

int c = 0;
int color = 64800;

int state = 1;
int last_state = 1;

unsigned long current_millis = 0;
unsigned long refresh_interval = 500;
unsigned long previous_refresh_millis = 0;

unsigned long testing_time = 10000;
unsigned long testing_interval = 500;
unsigned long previous_testing_millis = 0;

float starting_pressure = 0;
float pressure_leak = 0;
float max_leak = 1.00;
float max_pressure = 16.00;
unsigned long progress = 0;



void tcaselect(uint8_t i) {  //función para seleccionar el canal de I2C
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}


void setup() {
  Serial.begin(9600);   //inicia la comunicación serial a 9600 bps
  tft.begin(); //inicializa la comunicación con pantalla
  ts.begin(); //Starts touchscreen
  
  tft.setRotation(1);  //ROTACION 90° A LA DERECHA
  tft.fillScreen(ILI9341_BLACK);  //fondo pantalla negro
  tft.setTextSize(2);

  tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
  tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0, 50, 0));
  tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(25, 182);
  tft.println("Test");
  tft.setCursor(225, 182);
  tft.println("Config");
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(125, 182);
  tft.println("Send");

  tft.setTextColor(ILI9341_BLUE);  //texto color verde
  tft.setCursor(20, 20);
  tft.println("Dif. Pressure:");

  tft.setCursor(260, 20);
  tft.setTextSize(2);
  tft.println("mbar");


  Wire.begin();  //inicializa la comunicación I2C


  tcaselect(3);  //selecciona el canal 3 (SDP810)
  SDP800.begin();
  SDP800.startContinuousMeasurement(SDP_TEMPCOMP_DIFFERENTIAL_PRESSURE, SDP_AVERAGING_TILL_READ);
}

void loop() {
  //Touch Status
  if (ts.touched()){
    last_touch = current_touch;
    current_touch = 1;

    last_point = current_point;
    current_point = ts.getPoint();
  }
  else{
    last_touch = current_touch;
    current_touch = 0;
    last_point = current_point;
    current_point = ts.getPoint();

    

  }

  //Refresh Dif. Pressure value
  current_millis = millis();
  if (current_millis - previous_refresh_millis > refresh_interval){
    previous_refresh_millis = current_millis;
    tft.setTextColor(ILI9341_GREEN);
    tcaselect(3);  //selecciona el canal 3
    borrar2();
    //DP = (((SDP800.getDiffPressure()) / (100))/4);
    DP = random(0,10);
    tft.setCursor(190, 20);
    tft.print(DP);
    if (DP > max_pressure){
      last_state = state;
      state = ALARM;
    }
  }
  else if (current_millis - previous_refresh_millis < 0){
    previous_refresh_millis = 0;
  }

  switch(state){
    case STAND_BY:
          
          break;
    case INIT:
          //Touching buttons
          if (current_touch==1){
            // flip it around to match the screen.
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Test");
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
          }
          else if (current_touch==0 && last_touch==1){
            // flip it around to match the screen.
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Reset");
              starting_pressure = DP; 
              state = MEASURE;
              break;
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
          }
     
          break;
    case MEASURE:
          //Measure Status
          if (current_millis - previous_testing_millis > testing_interval){
            previous_testing_millis = current_millis;
            progress += testing_interval;
            tft.fillRect(0, 40 , 320, 20 , ILI9341_BLACK);
            tft.setTextColor(ILI9341_GREEN);
            tft.setCursor(20, 40);
            pressure_leak = starting_pressure - DP;
            tft.println((String)((int)(progress*100/testing_time))+"% Leak: "+(String)pressure_leak+" mbar");
          }
          else if (current_millis - previous_testing_millis < 0){
            previous_testing_millis = 0;
          }

          //Touching buttons
          if (current_touch==1){
            // flip it around to match the screen.
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Reset");
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
          }
          else if (current_touch==0 && last_touch==1){
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Test");
              tft.fillRect(0, 40 , 320, 20 , ILI9341_BLACK);
              state = INIT;
              progress = 0;
              break;
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
          }

          //Measure Results
          if (progress == testing_time){
            if (pressure_leak > max_leak)
            {
              tft.fillRoundRect(280, 40, 40, 20, 40/8, tft.color565(255, 0, 0));
              tft.setCursor(290, 42);
              tft.setTextColor(ILI9341_WHITE);
              tft.println("X");
            }
            else
            {
              tft.fillRoundRect(280, 40, 40, 20, 40/8, tft.color565(0, 255, 0));
              tft.setCursor(290, 42);
              tft.setTextColor(ILI9341_WHITE);
              tft.println("OK");
            }
            state = RESULTS;
            progress = 0;
            tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0,100,0));
            tft.setTextColor(ILI9341_WHITE);
            tft.setCursor(125, 182);
            tft.println("Send");
            
          }

          break;
    case RESULTS:
           //Touching buttons
          if (current_touch==1){
            // flip it around to match the screen.
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Test");
            }
            else if ((last_point.x >118 && last_point.x < 202)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(125, 182);
              tft.println("Send");
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(20,0,100));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
          }
          else if (current_touch==0 && last_touch==1){
            // flip it around to match the screen.
            int x_temp = last_point.x;            
            last_point.x = map(last_point.y, 0, 320, 320, 0);
            last_point.y = map(x_temp, 0, 240, 0, 240);

            if ((last_point.x >18 && last_point.x < 102)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(25, 182);
              tft.println("Test");
              tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0, 10, 0));
              tft.setTextColor(ILI9341_BLACK);
              tft.setCursor(125, 182);
              tft.println("Send");
              tft.fillRect(0, 40 , 320, 20 , ILI9341_BLACK);
              state = INIT;
              break;
            }
            else if ((last_point.x >118 && last_point.x < 202)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(125, 182);
              tft.println("Send");
              state = SEND;
              break;
            }
            else if ((last_point.x >218 && last_point.x < 302)&&(last_point.y >178 && last_point.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 100, 0));
              tft.setTextColor(ILI9341_WHITE);
              tft.setCursor(225, 182);
              tft.println("Config");
            }
            
          }
          break;
    case SEND:
          state = RESULTS;
          break;
    case SETTINGS:
          break;
    case SHUT_DOWN:
          break;
    case ALARM:
          tone(BUZZER_PIN, 420,250);
          if (ts.touched()){
            state = last_state;
            noTone(BUZZER_PIN);
          }
          break; 
    default:
          break;
  }
  
}

void borrar() {


  tft.fillRect(190, 20 , 30, 20 , ILI9341_BLACK);
  tft.fillRect(190, 60 , 40, 20 , ILI9341_BLACK);
  tft.fillRect(190, 100 , 50, 20 , ILI9341_BLACK);
  tft.fillRect(190, 140 , 50, 20 , ILI9341_BLACK);

  c++;

  if (c == 10)
  {
    color = 64800; //color naranjo
    changecolor();
  }

  if (c == 20)
  {
    color = 65535; //color blanco
    changecolor();
  }

  if (c == 30)
  {
    color = 63488; //color rojo
    changecolor();
  }

  if (c == 40)
  {
    color = 31; //color azul
    changecolor();
    c = 0;
  }
}


void borrar2() {

  tft.fillRect(190, 20 , 60, 20 , ILI9341_BLACK);
  tft.fillRect(190, 20 , 60, 20 , ILI9341_BLACK);
}

void changecolor() {
  tft.setTextColor(color);
  tft.setCursor(20, 20);
  tft.setTextSize(2);
  tft.print("Temperature: ");

  tft.setTextSize(1);
  tft.setCursor(260, 20);
  tft.print("o");
  tft.setTextSize(2);
  tft.println("C");

  tft.setCursor(20, 60);
  tft.setTextSize(2);
  tft.println("Humidity:");

  tft.setCursor(260, 60);
  tft.setTextSize(2);
  tft.println("%RH");

  tft.setCursor(20, 100);
  tft.setTextSize(2);
  tft.println("Altitude:");

  tft.setCursor(260, 100);
  tft.setTextSize(2);
  tft.println("m");

  tft.setCursor(20, 140);
  tft.setTextSize(2);
  tft.println("Air Pressure:");

  tft.setCursor(260, 140);
  tft.setTextSize(2);
  tft.println("atm");

  tft.setCursor(20, 180);
  tft.setTextSize(2);
  tft.println("IN Pressure:");

  tft.setCursor(260, 180);
  tft.setTextSize(2);
  tft.println("mbar");

  tft.setCursor(20, 220);
  tft.setTextSize(2);
  tft.println("OUT Pressure:");

  tft.setCursor(260, 220);
  tft.setTextSize(2);
  tft.println("mbar");
}

void serialdata()
{
  tcaselect(3);  //selecciona el canal 3
  Serial.print("OUT Pressure Diff: ");
  Serial.print(((SDP800.getDiffPressure()) / 100)/4);
  Serial.println(" mbar");
}
