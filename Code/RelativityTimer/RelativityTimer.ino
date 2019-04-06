/*
   Relativity Timer
   
   LCD Connections:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/

#include <LiquidCrystal.h>
#include <DueTimer.h>
#define R3_IN4 60 
#define R3_IN3 61
#define R3_IN2 68
#define R3_IN1 69

#define R4_IN4 65
#define R4_IN3 64
#define R4_IN2 63
#define R4_IN1 62

#define STD_TIME 60
#define LSPEED 300000.0 // km/sec
#define MAXSPEED 299000
#define NSPEEDS 11

int step1 = 0;
int step2 = 0;
int currentstate = 0;
int currentstate2 = 0;
volatile bool motorun = false;

int states[] = { B1000,
                 B1100,
                 B0100,
                 B0110,
                 B0010,
                 B0011,
                 B0001,
                 B1001 };

void initMotorPins() {
  pinMode(R3_IN4, OUTPUT);
  pinMode(R3_IN3, OUTPUT);
  pinMode(R3_IN2, OUTPUT);
  pinMode(R3_IN1, OUTPUT);
  pinMode(R4_IN1, OUTPUT);
  pinMode(R4_IN2, OUTPUT);
  pinMode(R4_IN3, OUTPUT);
  pinMode(R4_IN4, OUTPUT);
  REG_PIOA_OWER = 15;
  REG_PIOB_OWER = 15;
  REG_PIOB_ODSR = 0;
}

void motorStop(){
 REG_PIOA_ODSR = 0;
 REG_PIOB_ODSR = 0;
}

void motorStep2() {
      REG_PIOB_ODSR = states[currentstate2++] << 17;
      if (currentstate2 == 8)
        currentstate2 = 0;
}


void motorStep() {
      REG_PIOA_ODSR = states[currentstate++];
      if (currentstate == 8)
        currentstate = 0;
}

void mymotorStep() {
  motorStep();
  step1++;
  if (step1 >=4095) {
    Timer0.stop();
    Timer1.stop();
    motorun = false;
  }
}

void mymotorStep2() {
  motorStep2();
  step2++;
  if (step2 >=4095) {
    Timer1.stop();
    Timer0.stop();
    motorun = false;
  }
}

void resetStep2() {
  while (step2 < 4095) {
    motorStep2();
    delay(2);
    step2++;
  }
}


double relative_time(int time0, long speed1) {
  return time0 * sqrt(1 - (speed1 / LSPEED)*(speed1 / LSPEED));
}

double microsecs(double time0) {
  double k = 4096.0 / time0;
  return (1.0 / k)*1000000;
}

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
long speeds[] = {50000, 100000, 150000, 200000, 250000, 270000, 275000, 280000, 285000, 290000, 295000, MAXSPEED};
int currentspeed = 0;
double k,t,t1;

void setup() {
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  // Initialize Motor
  initMotorPins();
  Timer0.attachInterrupt(mymotorStep);
  Timer1.attachInterrupt(mymotorStep2);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Setup Positions");
  lcd.setCursor(0,1);
  lcd.print("Enter to end");
  while (digitalRead(8) == 0) {
     if (digitalRead(7)) {
       for (int i = 0; i < 50; i++)
         motorStep();
         delay(5);
     }
     if (digitalRead(6)) {
       for (int i = 0; i < 50; i++)
         motorStep2();
         delay(5); 
     }
  }
  while (digitalRead(8));
  lcd.clear();
 }

void loop() {
  int inc = digitalRead(6);
  int dec = digitalRead(7);
  int ent = digitalRead(8);

  if (inc == 1) { 
    currentspeed++;
    if (currentspeed > NSPEEDS)
      currentspeed = 0;
  }

  if (dec == 1) {
    currentspeed--;
    if (currentspeed < 0)
      currentspeed = NSPEEDS;
  }

  if (ent == 1 && !motorun) {
    k = relative_time(STD_TIME, speeds[currentspeed]);
    t = microsecs(k);
    t1 = microsecs(60); // standard minute
    Timer0.start(t);
    Timer1.start(t1);
    motorun = true;
    lcd.setCursor(0,1);
    lcd.print("Rel.Time:");
    lcd.print(k);
    lcd.print(" secs");
    while (digitalRead(8));
    while (motorun); 
    while (!digitalRead(8));
    lcd.clear();
    lcd.print("Resetting...");
    resetStep2();
    step1 = 0;
    step2 = 0;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Relativity Proj.");
  lcd.setCursor(0, 1);
  lcd.print("Speed:");
  lcd.print(speeds[currentspeed]);
  lcd.print("km/s");
  delay(100);
}
