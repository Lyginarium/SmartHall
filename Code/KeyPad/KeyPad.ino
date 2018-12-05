#include <Keypad.h>
#include <Wire.h>

const byte ROWS = 4; 
const byte COLS = 3; 
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; 
byte colPins[COLS] = {8, 7, 6}; 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

bool isSystemArmed;
bool isArmingProcedureActivated;
bool isDoorClosedAtTheBeginingOfArmingProcedure;
bool wasDoorOpenedDuringArmingProcedure;
bool wasDoorOpenedWhileSystemArmed;
bool firstWarningMessagePlayed;
bool secondWarningMessagePlayed;
char armingCode[] = "0203";
char disarmingCode[] = "0302";
const byte redLED = 9;
const byte greenLED = 10;
byte i = 0;
bool isDoorClosed;
unsigned long previousMillisLEDSwitch = 0;
unsigned long  armingProcedureCountdown = 0;
unsigned long previousMillisPlaybackWarningMessage = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  isSystemArmed = false;
  isArmingProcedureActivated = false;
  isDoorClosedAtTheBeginingOfArmingProcedure = false;
  wasDoorOpenedDuringArmingProcedure = false;
  wasDoorOpenedWhileSystemArmed = false;
  firstWarningMessagePlayed = false;
  firstWarningMessagePlayed = false;
  //Serial.println("Включение системы...  Система в работе.");
  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(1);        
  //Wire.write(x);             
  Wire.endTransmission();    
  //Serial.println("Текущий стаус системы: снято с охраны.");
  //Serial.println();
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
}
 
void loop()
{
  if (!isSystemArmed && !isArmingProcedureActivated) enterArmingCode ();
  if (isSystemArmed) 
    {
      enterDisarmingCode ();
      if(!isDoorClosed && !wasDoorOpenedWhileSystemArmed) 
      {
        wasDoorOpenedWhileSystemArmed = true;
        previousMillisPlaybackWarningMessage = millis();
      }
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillisLEDSwitch > 500 && wasDoorOpenedWhileSystemArmed) 
      {
       previousMillisLEDSwitch = currentMillis;  
       if (digitalRead(redLED)) digitalWrite(redLED, LOW);
       else digitalWrite(redLED, HIGH);
       }
       if(currentMillis - previousMillisPlaybackWarningMessage > 30000 && !firstWarningMessagePlayed && wasDoorOpenedWhileSystemArmed) 
      {
       //previousMillisLEDSwitch = currentMillis;  //Nahuya?!
       //Serial.println("Неизвестный пользователь. Пожалуйста, авторизуйтесь в системе!");
       //Serial.println();
       Wire.beginTransmission(8); 
       Wire.write(3);        // 
       Wire.endTransmission();
       firstWarningMessagePlayed = true;
       }
       if(currentMillis - previousMillisPlaybackWarningMessage > 120000 && !secondWarningMessagePlayed && wasDoorOpenedWhileSystemArmed) 
      {
       //previousMillisLEDSwitch = currentMillis;  //Nahuya?!
       //Serial.println("Объект находится под охраной! Идентифицируйте себя или немедленно покиньте помещение!");
       //Serial.println();
       Wire.beginTransmission(8); 
       Wire.write(4);        // 
       Wire.endTransmission();
       secondWarningMessagePlayed = true;
       }
    }
  if (!isSystemArmed && isArmingProcedureActivated && (isDoorClosed  ||  isDoorClosedAtTheBeginingOfArmingProcedure))
    {
     if(!isDoorClosedAtTheBeginingOfArmingProcedure) isDoorClosedAtTheBeginingOfArmingProcedure = true; 
     if(isDoorClosedAtTheBeginingOfArmingProcedure && !isDoorClosed && !wasDoorOpenedDuringArmingProcedure)
     {
      wasDoorOpenedDuringArmingProcedure = true;
      Wire.beginTransmission(8); 
      Wire.write(5);        // Счастливого пути!
      Wire.endTransmission();
      
     }
     if(wasDoorOpenedDuringArmingProcedure && isDoorClosed)
      {
        isSystemArmed = true;
        isArmingProcedureActivated = false;
        isDoorClosedAtTheBeginingOfArmingProcedure = false;
        wasDoorOpenedDuringArmingProcedure = false;
        digitalWrite(redLED, HIGH);
        digitalWrite(greenLED, LOW);
        tone(13, 1800, 1000);
        Wire.beginTransmission(8); 
        Wire.write(6);        // Система на охране
        Wire.endTransmission();
      }
      
     unsigned long currentMillis = millis();
     if(currentMillis - previousMillisLEDSwitch > 500 && !isSystemArmed) 
      {
       previousMillisLEDSwitch = currentMillis;  
       if (digitalRead(redLED)) digitalWrite(redLED, LOW);
       else digitalWrite(redLED, HIGH);
       if (digitalRead(greenLED)) digitalWrite(greenLED, LOW);
       else digitalWrite(greenLED, HIGH);
      }
     if(currentMillis - armingProcedureCountdown > 60000 && !isSystemArmed) //&& !wasDoorOpenedDuringArmingProcedure
      {
       isArmingProcedureActivated = false; 
       isDoorClosedAtTheBeginingOfArmingProcedure = false;
       wasDoorOpenedDuringArmingProcedure = false;
       tone(13, 1800, 1000);
       Wire.beginTransmission(8); 
       Wire.write(7);        // Превышено время ожидания. Отмена постановки на охрану.
       Wire.endTransmission();
      }
    } 
  if (!isSystemArmed && isArmingProcedureActivated && !isDoorClosed && !isDoorClosedAtTheBeginingOfArmingProcedure) 
    {
      tone(13, 1800, 1000);
      Wire.beginTransmission(8); 
      Wire.write(8);                   // При вводе кода постановки на охрану внутренняя дверь должна быть закрыта!
      Wire.endTransmission();
      isArmingProcedureActivated = false;
    }

  Wire.requestFrom(8, 1);    // request 1 byte from slave device #8
  while (Wire.available()) 
  { 
    isDoorClosed = Wire.read(); 
   }
}

void enterArmingCode ()
{
  char key = keypad.getKey();
  if (key != NO_KEY)
    {
     tone(13, 1800, 100);
     if (key == armingCode[i])
        {
         if (i == strlen(armingCode) - 1)
          {
          isArmingProcedureActivated = true;
          armingProcedureCountdown = millis();
          //Serial.println("Начата процедура постановки на охрану...");
          //Serial.println();
          Wire.beginTransmission(8); 
          Wire.write(9);        // 
          Wire.endTransmission();
          tone(13, 1800, 1000);
          i = 0;
          }
        else i++;
        }
     else if (i != 0) i = 0;
    }
}

void enterDisarmingCode ()
{
  char key = keypad.getKey();
  if (key != NO_KEY)
    {
     tone(13, 1800, 100);
     if (key == disarmingCode[i])
        {
         if (i == strlen(disarmingCode) - 1)
          {
          isSystemArmed = false;
          digitalWrite(greenLED, HIGH);
          digitalWrite(redLED, LOW);
          tone(13, 1800, 1000);
          Wire.beginTransmission(8); 
          Wire.write(10);        // 
          Wire.endTransmission();
          i = 0;
          wasDoorOpenedWhileSystemArmed = false;
          firstWarningMessagePlayed = false;
          secondWarningMessagePlayed = false;
          }
        else i++;
        }
     else if (i != 0) i = 0;
    }
}
