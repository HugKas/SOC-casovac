#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>

#define RELAY 5

DS3231 RTC;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
bool h12Flag;                    // 24h format
bool pmFlag;                     // 24h format
int Time[3];
const int NumberOfTimers = 5;
const int NumberOfTimerParameters = 4; // hour,minute,second,lamp mode
int Timers[NumberOfTimers][NumberOfTimerParameters] = {{
                                                           0,
                                                           2,
                                                           0,
                                                           3,
                                                       },
                                                       {
                                                           11,
                                                           50,
                                                           0,
                                                           0,
                                                       },
                                                       {
                                                           12,
                                                           38,
                                                           0,
                                                           1,
                                                       },
                                                       {
                                                           12,
                                                           39,
                                                           0,
                                                           2,
                                                       },
                                                       {
                                                           23,
                                                           59,
                                                           0,
                                                           3,
                                                       }}; // 4-th bit is to determine to which state will the lamp switch (0=off,1=Night,2=Half,3=Max))
bool Status[NumberOfTimers];


long epoch= 1735823520;//https://www.epochconverter.com/  - na nastavenie casu

void getTime();
void lcdWriteTime();
int compareTime(int Timer1ToCompare[], int Timer2ToCompare[], int TimeToCompare[]);
void setLampOff();
void setLampNight();
void setLampHalf();
void setLampMax();
void lampChange(int CurrentPosition);
void findClosestTimer();

void setup()
{
  //RTC.setEpoch(epoch); - na nastavenie casu
  for (int i=0;i<NumberOfTimers;i++) {
    Status[i] = false;
    }
  pinMode(RELAY, OUTPUT);
  lcd.init(); 
  Wire.begin();
  lcd.backlight();
  lcd.print("Hello");
 
  delay(2000);
}

void loop()
{
  getTime();
  lcdWriteTime();
  findClosestTimer();
  delay(100);
}

void getTime() //  get current time, and save it to Time array
{
  Time[0] = RTC.getHour(h12Flag, pmFlag);
  Time[1] = RTC.getMinute();
  Time[2] = RTC.getSecond();
}

void lcdWriteTime() // write Time on lcd
{
  lcd.setCursor(0, 1);

  if (Time[0] < 10)
  {
    lcd.print("0");
    lcd.print(Time[0]);
  }
  else
  {
    lcd.print(Time[0]);
  }
  lcd.print(":");

  if (Time[1] < 10)
  {
    lcd.print("0");
    lcd.print(Time[1]);
  }
  else
  {
    lcd.print(Time[1]);
  }
  lcd.print(":");

  if (Time[2] < 10)
  {
    lcd.print("0");
    lcd.print(Time[2]);
  }
  else
  {
    lcd.print(Time[2]);
  }
}

int compareTime(int Timer1ToCompare[], int Timer2ToCompare[], int TimeToCompare[])
{
  int comparison = 0;

  long Timer1Seconds = (long(Timer1ToCompare[0]) * 3600 + long(Timer1ToCompare[1] * 60) + long(Timer1ToCompare[2]));
  long Timer2Seconds = (long(Timer2ToCompare[0]) * 3600 + long(Timer2ToCompare[1] * 60) + long(Timer2ToCompare[2]));
  long TimeSeconds = (long(TimeToCompare[0]) * 3600 + long(TimeToCompare[1] * 60) + long(TimeToCompare[2]));

  if (TimeSeconds >= Timer1Seconds and TimeSeconds <= Timer2Seconds)
  {
    comparison = 1;
  }
  else
  {
    comparison = 0;
    return comparison;
  }
  return comparison;
}

void setLampOff()
{
  digitalWrite(RELAY, LOW);
}
void setLampNight()
{
  digitalWrite(RELAY, LOW);
  delay(5000);
  digitalWrite(RELAY, HIGH);
  delay(500);
  digitalWrite(RELAY, LOW);
  delay(500);
  digitalWrite(RELAY, HIGH);
  delay(500);
  digitalWrite(RELAY, LOW);
  delay(500);
  digitalWrite(RELAY, HIGH);
}
void setLampHalf()
{
  digitalWrite(RELAY, LOW);
  delay(5000);
  digitalWrite(RELAY, HIGH);
  delay(500);
  digitalWrite(RELAY, LOW);
  delay(500);
  digitalWrite(RELAY, HIGH);
}
void setLampMax()
{
  digitalWrite(RELAY, LOW);
  delay(5000);
  digitalWrite(RELAY, HIGH);;
}

void lampChange(int CurrentPosition)
{
  int State = Timers[CurrentPosition][3];
  if (State == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Stav 0-nesvieti");
    setLampOff();
  }
  else if (State == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Stav 1-noc     ");
    setLampNight();
  }
  else if (State == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("Stav 2-tlmene  ");
    setLampHalf();
  }
  else if (State == 3)
  {
    lcd.setCursor(0, 0);
    lcd.print("Stav 3-den     ");
    setLampMax();
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Chyba");
  }
}

void findClosestTimer()
{
  int TimerStart[3];
  int TimerEnd[3];
  for (size_t j = 0; j < NumberOfTimers; j++)
  {
    if (j == NumberOfTimers - 1)
    {
      for (int i = 0; i < 3; i++)
      {
        TimerStart[i] = Timers[j][i];
      }
      int polnoc[4] = {24, 0, 0, 0};
      if (compareTime(TimerStart, polnoc, Time) == 1)
      {
        if (Status[j] == false)
        {
          lampChange(j);
          for (int k = 0; k < NumberOfTimerParameters; k++)
          {
            Status[k] = false;
          }
          Status[j] = true;
          break;
        }
      }
    }
    else
    {
      for (int i = 0; i < 3; i++)
      {
        TimerStart[i] = Timers[j][i];
        TimerEnd[i] = Timers[j + 1][i];
      }

      if (compareTime(TimerStart, TimerEnd, Time) == 1)
      {
        if (Status[j] == false)
        {
          lampChange(j);
          for (int k = 0; k < NumberOfTimerParameters; k++)
          {
            Status[k] = false;
          }
          Status[j] = true;
          for (int k = 0; k < NumberOfTimerParameters; k++)
          {
          }
          break;
        }
      }
    }
  }
}
