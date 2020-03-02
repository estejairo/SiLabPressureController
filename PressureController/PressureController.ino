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

#define STAND_BY  0
#define INIT      1
#define MEASURE   2
#define RESULTS   3
#define SEND      4
#define SETTINGS  5
#define SHUT_DOWN 6

/*////// GLOBALS \\\\\\*/
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_FT6206 ts = Adafruit_FT6206();
SDP_Controller SDP800;

float DP;

int c = 0;
int color = 64800;

int state = 1;

unsigned long refresh_interval = 500;
unsigned long previous_millis = 0;



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

  tft.setTextColor(ILI9341_BLUE);  //texto color verde

  
  tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
  tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
  tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
  
  tft.setCursor(20, 20);
  tft.setTextSize(2);
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

  switch(state){
    case STAND_BY:
          
          break;
    case INIT:
          //Refresh Dif. Pressure value
          unsigned long current_millis = millis();
          if (current_millis - previous_millis > refresh_interval){
            previous_millis = current_millis;
            tft.setTextColor(ILI9341_GREEN);
            tcaselect(3);  //selecciona el canal 3
            
            borrar2();
            tft.setCursor(190, 20);
            tft.print(DP);
          }
          else if (current_millis - previous_millis < 0){
            previous_millis = 0;
          }

          //Touching buttons
          if (ts.touched()){
            TS_Point p = ts.getPoint();
            
            // flip it around to match the screen.
            int x_temp = p.x;            
            p.x = map(p.y, 0, 320, 320, 0);
            p.y = map(x_temp, 0, 240, 0, 240);

            if ((p.x >18 && p.x < 102)&&(p.y >178 && p.y < 222)){
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(20,0,100));
            }
            if ((p.x >118 && p.x < 202)&&(p.y >178 && p.y < 222)){
              tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(20,0,100));
            }
            if ((p.x >218 && p.x < 302)&&(p.y >178 && p.y < 222)){
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(20,0,100));
            }
          }
          else{
              tft.fillRoundRect(20, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
              tft.fillRoundRect(120, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
              tft.fillRoundRect(220, 180, 80, 40, 40/8, tft.color565(0, 20, 0));
              
          }
     
          break;
    case MEASURE:
          break;
    case RESULTS:
          break;
    case SEND:
          break;
    case SETTINGS:
          break;
    case SHUT_DOWN:
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
