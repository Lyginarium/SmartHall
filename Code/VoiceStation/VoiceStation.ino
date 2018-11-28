
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>
#include <Wire.h>

class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnCardOnline(uint16_t code)
  {
    Serial.println("Card online ");
  }
  static void OnCardInserted(uint16_t code)
  {
    Serial.println("Card inserted ");
  }
  static void OnCardRemoved(uint16_t code)
  {
    Serial.println("Card removed ");
  }
};

SoftwareSerial secondarySerial(10, 11); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

uint32_t lastPlay; // длительность проигрывания трека
uint16_t volumeTmp; // громкость
bool ReedRelayClose; // флаг состояни геркона: 1 - замкнут (дверь закрыта), 0 - разомкнут (дверь открыта)
bool isItDoorBell = false;
const int PlayButton = 2; // кнопка звонка: нажата/отпущена
const int MP3ModuleBusy = 3; // состояние МП3-плеера: занят/свободен
const int DoorLimitSwitch = 4; // геркон
const int PlaybackLED = 5; // индикация состояния МП3-плеера: занят/свободен


void setup() 
{
  pinMode (PlayButton, INPUT);
  pinMode (MP3ModuleBusy, INPUT);
  pinMode (DoorLimitSwitch, INPUT);
  pinMode (PlaybackLED, OUTPUT);
  mp3.begin(); // инициализация...
  mp3.setVolume(15);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
}

void loop() 
{
if ((digitalRead(PlayButton) ==  LOW)&& // Если нажата кнопка звонка и ничего не проигрывается, то:
    (digitalRead(MP3ModuleBusy) == HIGH))  
    {
      isItDoorBell = true;
      ReedRelayClose = digitalRead(DoorLimitSwitch); // запомнить, в каком состоянии геркон
      digitalWrite(PlaybackLED, HIGH); // зажечь индикатор активности плеера
      volumeTmp = mp3.getVolume(); // запомнить громкость
      mp3.setVolume(0); // убавить громкость до 0
      mp3.playFolderTrack(1, random(1, mp3.getFolderTrackCount(1) + 1)); // запустить воспроизведение, папка 01, случайный трек
    
      for(int i = 1; i <= volumeTmp; i++) // плавно вывести громкость вверх до установленной величины
        {
        mp3.setVolume(i);
        delay(100);
        }
        
    lastPlay = millis();
 
    }
    
 uint32_t now = millis(); // время с начала выполнения программы, мс
  if (((((now - lastPlay) > 90000)&&(digitalRead(MP3ModuleBusy) == LOW)) || ((digitalRead(DoorLimitSwitch) == LOW) && (ReedRelayClose))) && isItDoorBell)
  // если трек играет больше 90 секунд или дверь была закрыта и открылась
  {
    digitalWrite (PlaybackLED, LOW); // гасим индикатор активности плеера
    volumeTmp = mp3.getVolume();
        
    for(int i = (volumeTmp - 1); i >= 0; i--) // плавно убавляем звук до нуля
        {
        mp3.setVolume(i);
        delay(100);
        }
        
    mp3.stop();
    mp3.setVolume(volumeTmp);
    isItDoorBell = false;
   }
  
  mp3.loop(); // аптека, улица, фонарь  
}

void requestEvent() 
{
  Wire.write(digitalRead(DoorLimitSwitch)); // HIGH if the door is closed
 }

void receiveEvent(byte trackNumber) 
{
  while (Wire.available())  
  mp3.playFolderTrack(2, Wire.read());
}
