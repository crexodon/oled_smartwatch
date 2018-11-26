#include <Arduino.h> //using an Arduino Nano
#include <Wire.h>
#include <RTClib.h>
#include <U8g2lib.h>

U8G2_SSD1327_MIDAS_128X128_1_4W_SW_SPI u8g2(U8G2_R3, 13, 11, 10, 9, 8); //13 clock, 11 data, 10 cs, 9 dc, 8 reset
RTC_DS3231 rtc;

int button =0;
unsigned long buttonPushedMillis;
unsigned long displayTurnedOnAt;
const long turnOffDelay = 5000;
bool displayReady = false;
bool displayState = false;

void setup(){
  pinMode(2, INPUT_PULLUP);
  u8g2.begin();
  rtc.begin();
}

//prints a two digit number
//if a number is below 10, it adds a 0 before the number
//with offset you can variable the spacing between 0 and the number on the x axis
void printTwoDigit(int x, int y, int offset, int number){
  if(number < 10){
    u8g2.setCursor(x, y);
    u8g2.print("0");
    u8g2.setCursor(x + offset, y);
    u8g2.print(number, DEC);
  }
  else{
    u8g2.setCursor(x, y);
    u8g2.print(number, DEC);
  }
}

//prints the clock face
//x and y set the position from the center
//rad defines the radius of the circle
void printClockFace(int x, int y, int rad){
  u8g2.drawCircle(x, y, rad, U8G2_DRAW_ALL);
  for(int z = 0; z < 360; z = z + 30){
    float angle = z;
    angle = (angle * 71) / 4068;
    int x2 = (x + (sin(angle) * rad));
    int y2 = (y - (cos(angle) * rad));
    int x3 = (x + (sin(angle) * (rad - 5)));
    int y3 = (y - (cos(angle) * (rad - 5)));
    u8g2.drawLine(x2, y2, x3, y3);
  }
}

//prints the clock hands
//x and y set the position from the center
//rad defines the radius of the circle
void printClockHands(int x, int y, int rad, DateTime now){
  //seconds
  float angle = now.second() * 6;
  angle = (angle * 71) / 4068;
  int x2 = (x + (sin(angle) * rad));
  int y2 = (y - (cos(angle) * rad));
  u8g2.drawLine(x, y, x2, y2);

  //minutes
  angle = now.minute() * 6;
  angle = (angle * 71) / 4068;
  x2 = (x + (sin(angle) * (rad - 3)));
  y2 = (y - (cos(angle) * (rad - 3)));
  u8g2.drawLine(x, y, x2, y2);

  //hours
  angle = now.hour() * 30 + int((now.minute() / 12) * 6);
  angle = (angle * 71) / 4068;
  x2 = (x + (sin(angle) * (rad - 11)));
  y2 = (y - (cos(angle) * (rad - 11)));
  u8g2.drawLine(x, y, x2, y2);
}

//checks the button and cycles the display on/off
//turnOffDelay defines the on-time for the display
void checkButton(void){
  unsigned long currentMillis = millis();
  if(digitalRead(2) == LOW){
    buttonPushedMillis = currentMillis;
    displayReady = true;
  }

  if(displayReady){
    if((currentMillis - buttonPushedMillis) > 0){
      u8g2.setPowerSave(0);
      displayState = true;
      displayTurnedOnAt = currentMillis;
      displayReady = false;
    }
  }
  if(displayState){
    if((currentMillis - displayTurnedOnAt) >= turnOffDelay){
      displayState = false;
      u8g2.setPowerSave(1);
    }
  }
}

//checks if displayState is true and if the button is pressed
//if both are true, button gets incremented by 1
//this function is used to switch between the digital and analog clock
void checkClockFace(void){
  if(displayState == true && digitalRead(2) == LOW){
    button++;
  }
}

//prints the seconds in binary form
//experimental function with hardcoded parts to display it vertically and from bottom to top
void binarySeconds(int x, int y, int number){
  int a;
  int b;
  int z = 95;

  do{
    a = number % 2;
    b = number / 2;
    u8g2.setCursor(x, y + z);
    u8g2.print(a);
    z = z - 19;
  }while(z > 0);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y + 95);
  // u8g2.print(a);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y + 76);
  // u8g2.print(a);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y + 57);
  // u8g2.print(a);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y + 38);
  // u8g2.print(a);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y + 19);
  // u8g2.print(a);

  // a = number % 2;
  // b = number / 2;
  // u8g2.setCursor(x, y);
  // u8g2.print(a);
}

void loop(){
  checkButton();
  checkClockFace();
  DateTime now = rtc.now();

  //if button is moduloable by 2
  //if true it displays the digital clock
  if((button % 2) == 0){
    u8g2.firstPage();
    do{
      u8g2.setFont(u8g2_font_logisoso50_tn);
      printTwoDigit(0, 60, 33, now.hour());
      printTwoDigit(0, 118, 33, now.minute());
      u8g2.setFont(u8g2_font_helvR14_tn);
      binarySeconds(92, 22, now.second());
    } while(u8g2.nextPage());
  }

  //else it displays the analog clock
  else{
    u8g2.firstPage();
    do{
      printClockFace(64,64, 63);
      printClockHands(64, 64, 63, now);
    } while(u8g2.nextPage());
  }
}