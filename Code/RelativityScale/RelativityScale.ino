#include <HX711_ADC.h>
#include <LiquidCrystal.h>

#define MAXSPEED 299000
#define NSPEEDS 12
#define LSPEED 300000.0 // km/sec

//HX711 constructor (dout pin, sck pin):

const int dout = 10, sck = 9;

HX711_ADC LoadCell(dout, sck);

// LCD constructor

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Buttons

const int inp1 = 8, inp2 = 7, ent = 6;

long thetime, thetime2;

// Load cell calibration factor, as measured previously

const float c = 1067.42;

// Selectable speeds

long speeds[] = {0, 50000, 100000, 150000, 200000, 250000, 270000, 275000, 280000, 285000, 290000, 295000, MAXSPEED};
int currentspeed = 12;
float mass0 = 0, mass = 0;

float relativeMass(float mass0, long speed1) {
  return mass0/sqrt(1-(speed1/LSPEED)*(speed1/LSPEED));
}

void setup() {
  pinMode(inp1, INPUT);
  pinMode(inp2, INPUT);
  pinMode(ent, INPUT);
  lcd.begin(16,2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrating");
  lcd.setCursor(0,1);
  lcd.print("Scale...");
  LoadCell.begin();
  long stabilisingtime = 2000; 
  LoadCell.start(stabilisingtime);
  // Set calibration factor
  LoadCell.setCalFactor(c); 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibration");
  lcd.setCursor(0,1);
  lcd.print("done!");
  delay(2000);
  lcd.clear();
  printSpeed();
}

void printSpeed() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Speed:");
  lcd.print(speeds[currentspeed]);
  lcd.print("Km/s");
}

void loop() {
  int inc = digitalRead(inp1);
  int dec = digitalRead(inp2);
  int enter = digitalRead(ent);
  LoadCell.update();
  if (inc) {
    currentspeed ++;
    if (currentspeed > NSPEEDS)
      currentspeed = 0;
  }
  
  if (dec) {
    currentspeed --;
    if (currentspeed < 0)
      currentspeed = NSPEEDS;
  }

  if (enter) {
    currentspeed = 0;
    printSpeed();
  }
  if (millis()- thetime > 250) {
    mass0 = LoadCell.getData();
    if (mass0 < 1) {
      mass0 = 0;
      currentspeed = 0;
    }
    thetime = millis();
  }
  if (millis() - thetime2 > 2000) {
    mass = relativeMass(mass0, speeds[currentspeed]);
    printSpeed();
    lcd.setCursor(0,1);
    lcd.print("Mass: ");
    lcd.print(int(mass));
    lcd.print("g");
    currentspeed++;
    if (currentspeed > NSPEEDS)
      currentspeed = 0;
    thetime2 = millis();
  }
  delay(100);
}
