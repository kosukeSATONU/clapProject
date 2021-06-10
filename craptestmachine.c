#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); //TX&RX for MP3VoiceModule

int LED_pin = 13;
int SW_pin = 2;
int playTime_pin = 3; //busypin

unsigned char order[4] = {0xAA, 0x02, 0x00, 0xB0};
int playWait = 100;
int SWdata = 0;
int playTime = 0;

void setup()
{
    pinMode(SW_pin, INPUT);
    pinMode(LED_pin, OUTPUT);     //スイッチLED用セットアップ
    pinMode(playTime_pin, INPUT); //Acquire BUSYpin 演奏時HIGH

    Serial.begin(9600);
    mySerial.begin(9600);
    volume(0x10); //Volume settings 0x00-0x1E//ボリュームコントロール
}

void loop()
{

    SWdata = digitalRead(SW_pin);
    playTime = digitalRead(playTime_pin);

    if (SWdata == 0)
    { //スイッチHIGHでLEDHIGH
        digitalWrite(LED_pin, 0);
    }
    else
    {
        digitalWrite(LED_pin, 1);
        play(0x01);               //Play the specified audio:0x01-file0001//trackを指定
        mySerial.write(order, 4); //order play
    }

    Serial.print(playTime);
    Serial.print("  SWdata=");
    Serial.println(SWdata);

    delay(100);
}

void play(unsigned char Track)
{
    unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3};
    mySerial.write(play, 6);
}
void volume(unsigned char vol)
{
    unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
    mySerial.write(volume, 5);
}
