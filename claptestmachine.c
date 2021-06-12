#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); //TX&RX for MP3VoiceModule

int LED_pin = 13;
int SW_pin = 2;
int playing_pin = A3; //busypin

unsigned char order[4] = {0xAA, 0x02, 0x00, 0xB0};
int playWait = 100;
int checkSW = 0;
int playing = 0;
int playNum = 0;

void setup()
{
    pinMode(SW_pin, INPUT);
    pinMode(LED_pin, OUTPUT);    //スイッチLED用セットアップ
    pinMode(playing_pin, INPUT); //Acquire BUSYpin 演奏時HIGH

    Serial.begin(9600);
    mySerial.begin(9600);
    volume(0x10); //Volume settings 0x00-0x1E//ボリュームコントロール
}

void loop()
{

    checkSW = digitalRead(SW_pin);
    playing = analogRead(playing_pin); //450~455くらい

    //スイッチHIGHorLOWチェック
    if (checkSW == 0)
    {
        digitalWrite(LED_pin, 0);
        playNum = 0;
    }
    else if (checkSW == 1)
    {
        digitalWrite(LED_pin, 1);
        playNum = 1;
    }

    Serial.print("  playNum=");
    Serial.print(playNum);

    Serial.print("  checkSW=");
    Serial.print(checkSW);

    Serial.print("  playing=");
    Serial.println(playing);

    delay(100);
}

//音楽再生
void myPlaySound()
{

    if (playNum == 1)
    {
        play(0x01);               //Play the specified audio:0x01-file0001//trackを指定
        mySerial.write(order, 4); //order play
        delay(playWait);
    }
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
