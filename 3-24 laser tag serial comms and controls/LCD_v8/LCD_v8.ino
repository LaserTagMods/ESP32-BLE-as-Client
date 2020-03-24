
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
HardwareSerial SerialLCD( 1 );

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;
int variable = 0;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialLCD.begin(9600,SERIAL_8N1, 16, 17);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  lcd.print("hello, world!");
}
unsigned long startt = millis();
void loop() {
  // put your main code here, to run repeatedly:
  if (SerialLCD.available()) {
    String readStr = SerialLCD.readStringUntil('\n');
    Serial.println(readStr);
  }
  if (millis() - startt > 5000) {
    SerialLCD.println(variable);
    variable = variable+1;
    startt = millis();
  }
}
