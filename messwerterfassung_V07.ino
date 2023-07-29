// aktuelles Datum: siehe 2023
// error-codes: siehe:  (value=xx)
// Falls Datum / Uhrzeit nicht stimmen: stelle die RTC mit "RTC-Initialisierung-v6.ino"



#include <Wire.h>               // for I2C communication 
#include <LiquidCrystal_I2C.h>  // for LCD control
#include "virtuabotixRTC.h"    // hier manuell eingebunden
// Creation of the Real Time Clock Object
// Definition: virtuabotixRTC(uint8_t inSCLK, uint8_t inIO, uint8_t inC_E);
// in: virtuabotixRTC.h
virtuabotixRTC myRTC(6, 7, 8);
// Pin-Configuration:
// Port 6 = SLCK
// Port 7 = IO
// Port 8 = C_E

#include <SPI.h>
/*#include <SD.h>
// Create on object
File myFile;*/


#define PI 3.1415926

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

const int sensorPin = A0;           // analog input pin of ardiono

// Für die Kalibirierung notwendig:
const int adc_min = 53;            // Wert des ADC ohne Wasser = 4mA
const int adc_min_error = 48;      // Werte unterhalb dieser Schwelle stellen einen Fehler dar, z.B. Kontaktproblem Drucksensor
                                    // und Werte oberhalb dieser Schwelle und unterhlab adc_min werden als "0 Liter" interpretiert
const int adc_max = 1014;           // Wert bei 5m Höchststand = 20mA
const float h_max = 4.32;              // Maximal-Füllhöhe Zisterne


const int durchmesser = 200.0;    // Durchmesser Zisterne in cm
const int fuellstand_max = 10870; // Maximales Fassungsvermögen der Zisterne in Litern (bis zum Filter)
const int version_number = 3;     // Versionsnummer des Programms
const int chipSelect = 4;         // for SD-Card
  
float adc_average = 0.0;        // the average-value of the sensor-input
unsigned int fuellstand_liter = 0;
long fuellstand_prozent = 0;    // 4 Bytes:  muss long sein
float fuellhoehe_meter =0.00;   // Fuellhoehe Zisterne in Meter
String display_date_time_String;  // Date and time für LCD
String store_date_time_String;    // Date and time für SD-Card

byte error_code = 0;        // abgespeicherter Error-Code

int lastSensorValue = 0;

// ########################################################
//                 Unterfunktionen
// ########################################################
/*
void storeSensorValue(int sensorValue) {
  // store a measurement in the SD-Card. Format :
  //    >>25.04.2023;11:15;135;1
  //    legend:
  //    1.  date 
  //    2.  time
  //    3.  adc_value (raw-value)
  //    4.  error_code
  // Note: use of a ";" to divide the parts for better import in Excel via csv

  const byte threshold = 4;    // define the threshold value for the difference between current and last value to decide if the sensor-values should be stored
  const byte delta_time = 60;  // define the time in seconds for the minimal time-distance between two measurements

  // initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD card initialization failed"));
    error_code |= (1 << 2); // set the 3rd bit from the right to 1 (value=4)
    return;
  }

  // calculate the difference between the current and last sensor values
  int diff = abs(sensorValue - lastSensorValue);

  // check if the difference is higher than the threshold value
  if (diff >= threshold) {
    //  ########################################
    //  #### we have to store a measurement ####
    //  ########################################
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
   
    myFile = SD.open("Test.txt", FILE_WRITE);

    
    // if the file opened okay, write to it:
    if (myFile) {
      Serial.println(F("Writing to Test.txt..."));
      myFile.println(store_date_time_String + " ; " + sensorValue + " ; " + error_code);
      Serial.println(store_date_time_String + " ; " + sensorValue + " ; " + error_code);
      // close the file:
      myFile.close();
      Serial.println(F("......................done."));
      } 
      else {
      // if the file didn't open, print an error:
      Serial.println(F("error opening Test.txt"));
      error_code |= (1 << 3); // set the 4th bit from the right to 1  (value=8)
      }
    // update the last sensor value
    lastSensorValue = sensorValue;
   
    delay(delta_time*1000); // Wait a some time, i.e. delta_time=60 ==> wait at minimum 60 seconds
  }
} // end void
*/


// Diese Funktion liest die RTC aus, speicher es in "store_date_time_String" ab und gibt dies am seriellen Monitor aus
void RTC_auslesen() {
	myRTC.updateTime();

  int day = myRTC.dayofmonth;
  int month = myRTC.month;
  int year = myRTC.year;
  int hour = myRTC.hours;
  int minute = myRTC.minutes;
  int second = myRTC.seconds;

  
	Serial.print(F("Current Date / Time: "));
	Serial.print(day);      
	Serial.print(F("."));   
	Serial.print(month); 
	Serial.print(F("."));    
	Serial.print(year); 
	Serial.print(F(" // "));
	Serial.print(hour);           
	Serial.print(F(":"));
	Serial.print(minute);
	Serial.print(F(":")); 
	Serial.println(second);
  // Format: Current Date / Time: 1.1.2000 // 0:2:16


  // Einstellige Zeitangaben immer in zweistellige konvertieren:
  // Minute
  String minuteStr = String(minute, DEC);
  if (minuteStr.length() == 1) {
    minuteStr = "0" + minuteStr;
  }
  // Stunde
  String hourStr = String(hour, DEC);
  if (hourStr.length() == 1) {
    hourStr = "0" + hourStr;
  }
  // Tag
 String dayStr = String(day, DEC);
  if (dayStr.length() == 1) {
    dayStr = "0" + dayStr;
  }
  // Monat
  String monthStr = String(month, DEC);
  if (monthStr.length() == 1) {
    monthStr = "0" + monthStr;
  }

  // Display on LCD
  display_date_time_String = dayStr + "." + monthStr + "." + String(year) + "-" + hourStr + ":" + minuteStr;
  store_date_time_String = dayStr + "." + monthStr + "." + String(year) + ";" + hourStr + ":" + minuteStr;    // use the ";" inwstead of "-"
  Serial.println("store_date_time_String : >> " + store_date_time_String + " <<");
}




// Read the analog input pin, calculate the average
// and store the average value in "adc_average"
void readSensor() {
  const int numReadings = 50;     // Anzal der Messungen für die Mittelwertbildung, getestet bis 100
  int readings[numReadings];      // the readings from the analog input
  long total = 0;

  for (int i =0 ; i<numReadings; i++){
      readings[i] = analogRead(sensorPin);    // save the current reading
      total += readings[i];
      delay(50);
  }
  adc_average = total / numReadings;      // calculate the average
  Serial.println("\t  adc_average = " + String(adc_average));
  delay(1000);                         // wait for a moment
}




void printAverage() {
  lcd.setCursor(0,0);
  lcd.print("Average: ");
  lcd.print(adc_average);
  lcd.print(" mA     ");
  //Serial.println(F("\t  average = " + String(adc_average) + " ") ; 
}





// Diese Routine beschreibt das LCD mit den Messwerten
void printLCDparameters(){
  byte lcd_zeile = 0;          // Zeile des LCD, in die geschrieben wird (0-3)


 // 1. Zeile 
 lcd_zeile = 1-1;
  //lcd.setCursor(0,0);      // setCursor(Spalte, Zeile)
  //lcd.print("Version:      ");
  //lcd.setCursor(9,0);      // setCursor(Spalte, Zeile)
  //lcd.print(version_number);  
  lcd.setCursor(0,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(display_date_time_String);
  Serial.println(F("")) ; 
  Serial.println("\t  display_date_time_String = >>>" + display_date_time_String + "<<<");
    if (display_date_time_String = "") {
      // Fehlerfall: Kein Datum und keine Uhrzeit im display string
      Serial.println(F("......String Datum / Uhzeit ist leer!"));
      error_code |= (1 << 5); // set the 6rd bit from the right to 1 (value=32);
    }


  // 2. Zeile 
  lcd_zeile = 2-1;
  lcd.setCursor(0,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print("Stand= ");
  lcd.print(fuellhoehe_meter);
  lcd.setCursor(11,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(" m =     ");  
  //Serial.println(F("\t  Prozenbt ########### = " + String(fuellstand_prozent) + " %"));
  lcd.setCursor(15,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(fuellstand_prozent);
  lcd.setCursor(18,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print("% ");


  // 3. Zeile 
  lcd_zeile = 3-1;
  lcd.setCursor(0,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print("Rest =            ");
  lcd.setCursor(7,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(fuellstand_liter);

  lcd.setCursor(13,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print("Liter");



  // 4. Zeile: adc-Wert + Error-Code
  lcd_zeile = 4-1;
  lcd.setCursor(0,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print("adc  =        ");
  lcd.setCursor(7,lcd_zeile);      // setCursor(Spalte, Zeile)  
  lcd.print(round(adc_average));
  //Serial.print(F("\t LCD-average " + String(round(adc_average))) ; 
  lcd.setCursor(14,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(" e:   ");
  lcd.setCursor(17,lcd_zeile);      // setCursor(Spalte, Zeile)
  lcd.print(error_code);
  delay(50);
  
}


// function to initialize LCD
void lcd_initialization() {
  // initialize LCD with backlight on and cursor off
  lcd.init();
  lcd.backlight();
  lcd.noCursor();
}



// function to display the starttext on LCD
void lcd_starttext() {
  // display LCD Start-Text
  delay(100);
  lcd.setCursor(0,0);  // setCursor(Spalte, Zeile)
  lcd.print("Hello, world!       ");

  lcd.setCursor(0,1);
  lcd.print("Version 27.07.2023  ");

  lcd.setCursor(0,2);
  lcd.print("With Arduino Uno    ");
  
  lcd.setCursor(0,3);
  lcd.print("Power F. Diegmueller");

  delay(1000);                         // wait for a moment

  // clear the complete display
  //#lcd.setCursor(0,0);  // setCursor(Spalte, Zeile)
  //#lcd.print("                    ");

  //lcd.setCursor(0,1);  // setCursor(Spalte, Zeile)
  //lcd.print("                    ");

  //lcd.setCursor(0,2);  // setCursor(Spalte, Zeile)
  //lcd.print("                    ");

  //lcd.setCursor(0,3);  // setCursor(Spalte, Zeile)
  //lcd.print("                    ");

}


// ########################################################
//                           Setup
// ########################################################
void setup()
{
  delay(200);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  } // initialize LCD with backlight on and cursor off
  Serial.println(F("BEGIN of Setup-Routine"));
  lcd_initialization(); // call lcd_initialization() function to initialize LCD
  lcd_starttext();  // display the starttext on the LCD

 
  // SD-Card-handling
 /* Serial.print(F("Initializing SD card..."));
  if (!SD.begin(chipSelect)) {
    error_code |= (1 << 4); // set the 5th bit from the right to 1 (value=16)
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));
  Serial.println(F("END of Setup-Routine"));*/
}





// ########################################################
//                           Loop
// ########################################################
void loop() {

  delay(50);
  Serial.println(F("START of loop-Routine"));
  readSensor();
  Serial.println(F("...nach readSensor"));
  // Ermittlung Füllhöhe in Meter, wird in "fuellhoehe_meter" gespeichert
  if (adc_average < adc_min_error) {
    // Fehlerfall: Sensorkontakt evtl. fehlerhaft
    Serial.println(F("Value is lower than adc_min_error. Sensorkontakt Fehler ?"));
    error_code |= (1 << 0); // set the 1rd bit from the right to 1 (value=1)
    fuellhoehe_meter = -1;
  } else if (adc_average >= adc_min_error && adc_average < adc_min) {
    // Nehme dies noch als 0 Liter Füllmenge an, bzw. 0 m Füllstand
    fuellhoehe_meter = 0;
  } else if (adc_average > adc_max) {
    // Zisterne kurz vor Überlaufen
    fuellhoehe_meter = h_max/(adc_max-adc_min) * adc_average -  h_max/(adc_max-adc_min) * adc_min;
    error_code |= (1 << 1); // set the 2d bit from the right to 1 (value=2)
  } else {
    fuellhoehe_meter = h_max/(adc_max-adc_min) * adc_average -  h_max/(adc_max-adc_min) * adc_min;
  }


  // Ermittlung Füllstand in Liter
  fuellstand_liter = durchmesser/10.0 * durchmesser/10.0/4.0 * PI * fuellhoehe_meter * 10.0;
  
  // Ermittlung Füllstand in Prozent
  fuellstand_prozent = (long)(fuellstand_liter * 100.0L / fuellstand_max); // dies müssen "long" Variablen sein
  // Ausgaen auf dem seriellen Monitor  
  Serial.println(F("---Ausgabe serieller Monitor mit dem Messwerten : "));
  Serial.println("\t Füllhöhe in meter: " + String(fuellhoehe_meter));
  Serial.println("\t Füllstand in Liter: " + String(fuellstand_liter));
  Serial.println("\t durchmesser: " + String(durchmesser));
  Serial.println("\t  in Prozent = " + String(fuellstand_prozent) + " %");
  Serial.println("\t  in Berechnung1 = " + String(fuellstand_liter));
  Serial.println("\t  in Berechnung2 = " + String(fuellstand_liter * 100.0));



  // Auslesen der RTC und Ausgabe auf dem seriellen Monitor
  RTC_auslesen();
 
  printLCDparameters();
  // check if sensor-value (adc_average) should be stored to SD-Card
  /*storeSensorValue(adc_average);*/
  delay(200);                         // wait for a moment
}
