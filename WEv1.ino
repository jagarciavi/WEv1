/*
 * Author: José Antonio García (@jagarciavi)
 * Version: MEv1 v0.1 build 68
 */

 /*
  * Used libraries: LiquidCrystal Built-In, DHT sensor library, Adafruit Unified Sensor and Adafruit DHT Unified
  * Used examples: HelloWorld (LiquidCrystal) and DHT_Unified_Sensor (Adafruit DHT Unified).
  */

/* 
 *  Symbols from http://www.instructables.com/id/Clock-with-termometer-using-Arduino-i2c-16x2-lcd-D/
 *  Internal temp code from http://www.luisllamas.es/2015/09/usar-el-sensor-de-temperatura-interno-de-arduino/
 *  Button select code from http://www.dfrobot.com/wiki/index.php?title=Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)#Example_use_of_LiquidCrystal_library
 *  Thanks to Timofte Andrei, Luis Llamas, Arduino, DFRobot and Adafruit.  
 */

#include <LiquidCrystal.h>
#include <DHT.h>
#include <DHT_U.h>

int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define DHTPIN    2
#define DHTTYPE   DHT22


byte term[8] = {
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

byte drop[8] = //icon for water droplet
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110,
};

byte all[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

float temp = 0.0 / 0.0;
float hum = 0.0 / 0.0;

String ver = "v0.1 b68";
String hour = "HH:ii";
String date = "dd/mm/yy";
String sdate = "26 Jan";

boolean celsius = true;
boolean internal = false;
boolean checkTime = false;

void setup() {
  Serial.begin(9600); 
  dht.begin();
  sensor_t sensor;
  delayMS = sensor.min_delay / 1000;
  lcd.begin(16, 2);
  lcd.clear();
  printIntro();
}

void loop() {
  if (checkTime) {
    printTime();
  } else {
    printType();
  }
  drawTerm();
  drawDrop();
  if (internal) {
    temp = (float) getIntTemp();
    hum = 0 / 0.0;
  } else {
    temp = getExtTemp();
    hum = getExtHum();
  }
  printTemp(temp);
  printHum(hum);
  lcd_key = read_LCD_buttons();
  if (lcd_key == btnSELECT) {
    /*lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("MENU");
      lcd.setCursor(0, 1);
      lcd.print("<");
      lcd.setCursor(15,1);
      lcd.print(">");
      delay(5000);
      lcd.clear();*/
    if (celsius) {
      lcd.setCursor(7, 1);
      lcd.print("F");
      celsius = false;
    } else {
      lcd.setCursor(7, 1);
      lcd.print("C");
      celsius = true;
    }
  } else if (lcd_key == btnLEFT) {
    if (internal) {
      internal = false;
    } else {
      internal = true;
    }
  } else if (lcd_key == btnRIGHT) {
    if (checkTime) {
      checkTime = false;
    } else {
      checkTime = true;
    }
  }
  delay(30);
}

void printIntro() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WEv1 station (");
  lcd.print((char)226);
  lcd.print(")");
  lcd.setCursor(0, 1);
  lcd.print("Version ");
  lcd.print(ver);
  delay(4000);
  lcd.setCursor(0, 1);
  lcd.print("                   ");
  for (int i = 0; i < 15; i++) {
    lcd.createChar(2, all);
    lcd.setCursor(i, 1);
    lcd.write((byte)2);
    delay(100);
  }
  lcd.clear();
}

void printType() {
  lcd.setCursor(0, 0);
  if (internal) {
    lcd.print("Internal");
  } else {
    lcd.print("External");
  }
  lcd.setCursor(10, 0);
  lcd.print(sdate);
}

void printTime() {
  lcd.setCursor(0, 0);
  lcd.print("               ");
  lcd.setCursor(0, 0);
  lcd.print(date);
  lcd.setCursor(11, 0);
  lcd.print(hour);
}

void drawTerm() {
  lcd.createChar(0, term);
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
}

void drawDrop() {
  lcd.createChar(1, drop);
  lcd.setCursor(9, 1);
  lcd.write((byte)1);
}

void printTemp(float t) {
  lcd.setCursor(2, 1);

  if (!celsius) t = (t * 9) / 5 + 32;

  if (!isnan(t)) {
    lcd.print(t, 1);
  } else {
    lcd.print("----");
  }
  lcd.print((char)223);
  if (celsius) {
    lcd.setCursor(7, 1);
    lcd.print("C");
  } else {
    lcd.setCursor(7, 1);
    lcd.print("F");
  }
}

void printHum(float h) {
  lcd.setCursor(11, 1);
  if (!isnan(h)) {
    if (h == 0) {
      lcd.print(h, 2);
    } else {
      lcd.print(h, 1);
    }
  } else {
    lcd.print("----");
  }
  lcd.print("%");
}

double getIntTemp(void)
{
  unsigned int wADC;
  double t;

  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);
  delay(20);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  wADC = ADCW;

  // Esto es la función a calibrar.
  t = (wADC - 324.31 ) / 1.22;
  delay(100);
  return (t);
}

float getExtTemp() {
  //delay(delayMS);
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.print(event.temperature);
    Serial.println(" *C");
  }
  return event.temperature;
}

float getExtHum() {
  //delay(delayMS);
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(event.relative_humidity);
    Serial.println("%");
  }
  return event.relative_humidity;
}

int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;

  // For V1.0 comment the other threshold and use the one below:
  /*
    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 195)  return btnUP;
    if (adc_key_in < 380)  return btnDOWN;
    if (adc_key_in < 555)  return btnLEFT;
    if (adc_key_in < 790)  return btnSELECT;
  */


  return btnNONE;  // when all others fail, return this...
}


