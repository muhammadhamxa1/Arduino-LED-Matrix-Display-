/*  Arduino LED Matrix Display - 10x MAX7219 8x8 display-modules   *
 *  Pixels 80x8 - Digital Clock - Thermometer - Ticker             * 
 *  Dev: Muhammad Hamza - Date: 20/3/17 - Ver. 1.0             * 
 *  Time, Date, Ticker Text and brightness level can be controlled * 
 *  from bluetooth module and computer-android application         */

//Libraries
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <dht.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

//Constants
SoftwareSerial btSerial(3, 4); // RX, TX
const int photoCell = A0;
dht DHT;
const int DHT22_PIN = 9; // Data pin of DHT 22 (AM2302)
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 10;
const int numberOfVerticalDisplays = 1;
const int wait = 20; // In milliseconds
const int spacer = 1;
const int width = 5 + spacer; // The font width is 5 pixels
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

//Variables
int chk, length, brLevel, photoCellValue, count=0;
float hum, temp;
String msg;
boolean autoBR=true;
boolean messageCompleted=false;
boolean newMessage=false;
String tickerText = "muhammadhamxa1@gmail.com Made in pakistan!";
char incomingByte; 
String command;
unsigned long previousMillis = 0;        // will store last time LED was updated

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  rtc.begin();
  //rtc.adjust(DateTime(2017, 03, 21, 19, 47, 0));   // <----------------------SET TIME AND DATE: YYYY,MM,DD,HH,MM,SS
  matrix.setIntensity(1); // Use a value between 0 and 15 for brightness
  // Adjust to your own needs
  matrix.setPosition(0, 9, 0); // The first display is at <0, 0>
  matrix.setPosition(1, 8, 0); // The second display is at <1, 0>
  matrix.setPosition(2, 7, 0); // The third display is at <2, 0>
  matrix.setPosition(3, 6, 0); // And the last display is at <3, 0>
  matrix.setPosition(4, 5, 0); // And the last display is at <3, 0>
  matrix.setPosition(5, 4, 0); // And the last display is at <3, 0>
  matrix.setPosition(6, 3, 0); // And the last display is at <3, 0>
  matrix.setPosition(7, 2, 0); // And the last display is at <3, 0>
  matrix.setPosition(8, 1, 0); // And the last display is at <3, 0>
  matrix.setPosition(9, 0, 0); // And the last display is at <3, 0>
}

void loop() {
  unsigned long currentMillis = millis();
  communication();
  //Update thermometer and photoCell variables and on starting
  if (count==0){
    chk = DHT.read22(DHT22_PIN);
    //Read data and store it to variables hum and temp
    hum = DHT.humidity;
    temp= DHT.temperature;
    controlBR();
  }
  //Show content 
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    count++; //Seconds
  }
  //First display the clock for 5sec
  if (count>=0 && count <5){
    time();
  }
  //Then the day of the week for 3sec
  else if (count>=5 && count<8){
    today();
  }
  //Then the date for 3sec
  else if (count>=8 && count<11){
    date();
  }
  //Then the temperature for 3sec
  else if (count>=11 && count<14){
    temperature();
  }
  //Then the humidity for 3sec
  else if (count>=14 && count<17){
    humidity();
  }
  //Finally the ticker text
  else if (count>=17){
    scroll();
    count=0; //<----reset
  }
}
//Bluetooth communication
void communication(){
  if (btSerial.available()){
    incomingByte = btSerial.read();
    if(incomingByte=='>'){
       messageCompleted=true;
       newMessage=false;
    }
    else if (incomingByte=='<'){
       newMessage=true;
    }

    if (newMessage){
       command.concat(incomingByte);
    }
  }

  if(messageCompleted){
    //Brightness level
    if (command.charAt(1)=='B'){
      if (command.substring(2)=="Auto"){
        autoBR=true;
      }
      else{
        autoBR=false;
        brLevel= (command.substring(2)).toInt() - 1;
        
      }
    }
    //Update clock
    else if (command.charAt(1)=='T'){
      int h = (command.substring(2,4)).toInt();
      int m = (command.substring(5,7)).toInt();
      int s = (command.substring(8,10)).toInt();
      int D = (command.substring(11,13)).toInt();
      int M = (command.substring(14,16)).toInt();
      int Y = (command.substring(17,21)).toInt();
      rtc.adjust(DateTime(Y,M,D,h,m,s)); // <----------------------SET TIME AND DATE: YYYY,MM,DD,HH,MM,SS
    }
    //Update ticker text
    else if (command.charAt(1)=='t'){
      tickerText=command.substring(2);   
      Serial.println(tickerText);   
    }
    command="";
    messageCompleted=false;
  }
}
//Control brightness - It will run on every complete loop!
void controlBR(){
  if (autoBR){
    photoCellValue = analogRead(photoCell);
    photoCellValue = map(photoCellValue,1023,0,0,15); 
    matrix.setIntensity(photoCellValue); // Use a value between 0 and 15 for brightness
  }
  else{
    matrix.setIntensity(brLevel); // Use a value between 0 and 15 for brightness
  }
}
//Print time
void time(){
   DateTime now = rtc.now();
   int HH = now.hour();
   int MM = now.minute();
   int SS = now.second();
   if (HH<10){
     msg = "0" + String(HH) + ":"; 
   }
   else{
     msg = String(HH) + ":";
   }
   if (MM<10){
     msg += "0" + String(MM) + ":";
   }
   else{
     msg += String(MM) + ":";
   }
   if (SS<10){
     msg += "0" + String(SS);
   }
   else{
     msg += String(SS);
   }
   length = msg.length() * width;
   //fill with blank spaces the rest of the display area
   for (int i = numberOfHorizontalDisplays*8-length; i>0; i--){
     msg += " ";
   }
   matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); // Center text 
   matrix.fillScreen(LOW);
   matrix.print(msg);
   matrix.write();
}
//Print day of week
void today(){
  DateTime now = rtc.now();
  //Day of week
  msg = daysOfTheWeek[now.dayOfTheWeek()];
  length = msg.length() * width;
  //fill with blank spaces the rest of the display area
  for (int i = 8; i>0; i--){
    msg += " ";
  }
  matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); // Center text 
  matrix.fillScreen(LOW);
  matrix.print(msg);
  matrix.write();
}
//Print date
void date(){
  DateTime now = rtc.now();
  //Date:
  int dd = now.day();
  int mm = now.month();
  int yyyy = now.year();
  if (dd<10){
    msg = "0" + String(dd) + ":"; 
  }
  else{
    msg = String(dd) + ":";
  }
  if (mm<10){
    msg += "0" + String(mm) + ":";
  }
  else{
    msg += String(mm) + ":";
  }
  msg += String(yyyy);
  length = msg.length() * width;
  //fill with blank spaces the rest of the display area
  for (int i = 8; i>0; i--){
    msg += " ";
  }
  matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); // Center text 
  matrix.fillScreen(LOW);
  matrix.print(msg);
  matrix.write();
}
//Print temp and humidity
void temperature() {
    msg = "Temp: " + String(temp) +"oC";
    length = msg.length() * width;
    //fill with blank spaces the rest of the display area
    for (int i = 8; i>0; i--){
      msg += " ";
    }
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); // Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
}
void humidity(){
    msg = "Hum: " + String(hum) +"%";
    length = msg.length() * width;
    //fill with blank spaces the rest of the display area
    for (int i = 8; i>0; i--){
      msg += " ";
    }
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); // Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
}

//Ticker Text
void scroll(){
  for ( int i = 0 ; i < width * tickerText.length() + matrix.width() - 1 - spacer; i++ ) {

    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < tickerText.length() ) {
        matrix.drawChar(x, y, tickerText[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display

    delay(wait);
  }
}
//Control funcs. for display - Call them in void loop to test the LEDs
void fullOn(){
  matrix.fillScreen(HIGH);
  matrix.write();
}
void fullOff(){
  matrix.fillScreen(LOW);
  matrix.write();
}
