#include <LowPower.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <iarduino_RTC.h>


//value of humidity sensor in air
#define AIR_VALUE 836
//value of humidity sensor in water
#define WATER_VALUE 420
//when humidity percent falls lower than this value, start watering
#define CRITICAL_PERCENT 50
//stop watering when reaches this value
#define ENOUGH_PERCENT 80
//minimal temperature
#define MIN_TEMP 19

//each cell has a humidity sensor and a water valve
struct PlantCell {
  int hSensorPin;
  int waterValve;
  bool valveIsOpen;
  int humidityPercent;
  bool timeRegulated;
  PlantCell(int _hs, int _wv, bool _tr) {
    hSensorPin = _hs;
    waterValve = _wv;
    timeRegulated = _tr;
    valveIsOpen = false;
  }
  bool ReadHumidity() {
    int h_buff = humidityPercent;
    humidityPercent = map(analogRead(hSensorPin), AIR_VALUE, WATER_VALUE, 0, 100);
    return h_buff != humidityPercent;
  }
};

//morning watering start hr and min
int mHourOn = 9;
int mMinuteOn = 0;
//morning watering end hr and min
int mHourOff = 12;
int mMinuteOff = 0;
//evening watering start hr and min
int eHourOn = 17;
int eMinuteOn = 0;
//evening watering end hr and min
int eHourOff = 19;
int eMinuteOff = 0;
//current temperature
int currentTemp = 0;

//pins for LCD
const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
//thermometer pin
#define ONE_WIRE_BUS 1

//Cells
int nCells = 1;
PlantCell Cells [] = {PlantCell(A0, 2, false), PlantCell(A0, 4, true)};

//LCD
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
//Clock
iarduino_RTC time(RTC_DS1307);
//Thermometer
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int readCellHumidity(int i);
void printHumidity(int i, int value);
bool TimeIsBetween(int aHour, int aMinute, int bhour, int bminute, int chour, int cminute);
bool TimeIsEarlier(int aHour, int aMinute, int bhour, int bminute);

void setup () {
  Serial.begin(115200);
  time.begin();
  for (int i = 0; i < nCells ; i++)
  {
    pinMode(Cells[i].waterValve, OUTPUT);
  }
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("UUYCTOB JlOX");
  delay(3000);
  lcd.clear();
}

void loop() {
  time.gettime();
  sensors.requestTemperatures();
  if (currentTemp != sensors.getTempCByIndex(0))
  {
    lcd.setCursor(13, 0);
    currentTemp = sensors.getTempCByIndex(0);
    lcd.print(currentTemp);
    Serial.print(currentTemp);
    lcd.print("C");
  }
  if(currentTemp < MIN_TEMP){
    //TODO:
    //START HEATING
  }
  for (int i = 0; i < nCells; i++) {
    Cells[i].ReadHumidity();
    printHumidity(i, Cells[i].humidityPercent);
  }
  for (int i = 0; i < nCells; i++)
  {
    //if the cell isnt time regulated OR it's watering time
    if (Cells[i].timeRegulated == false || (Cells[i].timeRegulated && (TimeIsBetween(mHourOn, mMinuteOn, time.Hours, time.minutes, mHourOff, mMinuteOff) || TimeIsBetween(eHourOn, eMinuteOn, time.Hours, time.minutes, eHourOff, eMinuteOff))))
    {
      //if soil moisture too low and valve is closed, open valve
      if (!Cells[i].valveIsOpen && (Cells[i].humidityPercent <= CRITICAL_PERCENT)) {
        //SHEVA, CHANGE HIGH TO LOW ???
        digitalWrite(Cells[i].waterValve, HIGH);
        Cells[i].valveIsOpen = true;
        Serial.print("Opened  ");
        Serial.print(i);
        Serial.print(", humidity = ");
        Serial.print(Cells[i].humidityPercent);
        Serial.print('\n');
      }
    }
    //if valve open and moisture is high enough, close valve
    if (Cells[i].valveIsOpen && (Cells[i].humidityPercent >= ENOUGH_PERCENT)) {
      //SHEVA, CHANGE HIGH TO LOW ???
      digitalWrite(Cells[i].waterValve, LOW);
      Cells[i].valveIsOpen = false;
      Serial.print("Closed  ");
      Serial.print(i);
      Serial.print(", humidity = ");
      Serial.print(Cells[i].humidityPercent);
      Serial.print('\n');
    }
  }
  //sleep for 8 seconds
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void printHumidity(int i, int value) {
  Serial.print("Cell ");
  Serial.print(i);
  Serial.print(", value: ");
  Serial.print(value);
  Serial.print('\n');
//  int pos1 = 0;
//  int pos2 = 0;
//  switch (i) {
//    case 0:
//      pos1 = pos2 = 0;
//      break;
//    case 1:
//      pos1 = 6;
//      pos2 = 0;
//      break;
//    case 2:
//      pos1 = 0;
//      pos2 = 1;
//      break;
//    case 3:
//      pos1 = 6;
//      pos2 = 1;
//      break;
//  }
//  lcd.setCursor(pos1, pos2);
//  lcd.print(i + 1);
//  lcd.print(":");
//  lcd.print(value);
//  lcd.print("% ");
}

int readCellHumidity(int i) {
  return map(analogRead(Cells[i].hSensorPin), AIR_VALUE, WATER_VALUE, 0, 100);
}

bool TimeIsBetween(int aHour, int aMinute, int bhour, int bminute, int chour, int cminute) {
  //TEST
  return true;
  //return TimeIsEarlier(aHour, aMinute, bhour, bminute) && TimeIsEarlier(bhour, bminute, chour, cminute);
}

bool TimeIsEarlier(int aHour, int aMinute, int bhour, int bminute) {
  return (aHour < bhour || (bhour == bhour && aMinute < bminute));
}
