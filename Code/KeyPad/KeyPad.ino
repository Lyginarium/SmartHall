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
char armingCode[] = "0203";
char disarmingCode[] = "0302";
const byte redLED = 9;
const byte greenLED = 10;
byte i = 0;
bool isDoorClosed;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  isSystemArmed = false;
  Serial.println("Sysrem Disarmed");
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
}
 
void loop()
{
  if (isSystemArmed) systemDisarming ();
  else systemArming ();

  Wire.requestFrom(8, 1);    // request 1 byte from slave device #8

  while (Wire.available()) { // slave may send less than requested
    isDoorClosed = Wire.read(); // receive a byte
    }
    
  if(isDoorClosed) Serial.println("Door is closed");
  else Serial.println("Door is opened!");

}

void systemArming ()
{
  char key = keypad.getKey();
  if (key != NO_KEY)
    {
     tone(13, 1800, 100);
     if (key == armingCode[i])
        {
         if (i == strlen(armingCode) - 1)
          {
          isSystemArmed = true;
          digitalWrite(greenLED, LOW);
          digitalWrite(redLED, HIGH);
          Serial.println("System Armed");
          tone(13, 1800, 1000);
          i = 0;
          }
        else i++;
        }
     else if (i != 0) i = 0;
    }
}

void systemDisarming ()
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
          Serial.println("System Disarmed");
          tone(13, 1800, 1000);
          i = 0;
          }
        else i++;
        }
     else if (i != 0) i = 0;
    }
}
