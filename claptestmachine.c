#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); //TX&RX for MP3VoiceModule

int LED_pin = 13;
int SW_pin = 2;
int playing_pin = A3; //busypin

#define PUSH_SHORT 100
#define PUSH_LONG 32000
uint16_t count_low = 0;

unsigned char order[4] = {0xAA, 0x02, 0x00, 0xB0};
int playWait = 100;
int checkSW = 0;
int playing = 0;
int changeNum = 0;
int buttonState = 0;

void setup()
{
    pinMode(SW_pin, INPUT);
    pinMode(LED_pin, OUTPUT);    //スイッチLED用セットアップ
    pinMode(playing_pin, INPUT); //Acquire BUSYpin 演奏時HIGH

    Serial.begin(9600);
    mySerial.begin(9600);
    volume(0x1E); //Volume settings 0x00~0x1E//ボリュームコントロール
}

void loop()
{

    checkSW = digitalRead(SW_pin);
    playing = analogRead(playing_pin); //450~455くらい

    //入力待ち スイッチ押されるまでなにもしない
    if (checkSW == 1)
    {
        //digitalWrite(LED_pin, 1);
        buttonState = 1;
        changeNum = 0;
    }

    //スイッチHIGHorLOWチェック
    if (buttonState == 1)
    {
        digitalWrite(LED_pin, 1);
        changeNum = 1;
        if (playing == 0)
        {
            play(2); //play(トラック番号) 1=長い拍手/2=単拍手

            mySerial.write(order, 4); //order play
            //delay(100);
            // if (checkSW != 1)
            // {
            //     stop(04);
            // }
        }
        buttonState = 0;
    }
    else
    {
        digitalWrite(LED_pin, 0);
        changeNum = 0;
    }

    Serial.print("  changeNum=");
    Serial.print(changeNum);

    Serial.print("  checkSW="); //スイッチチェック
    Serial.print(checkSW);

    Serial.print("  playing="); //プレイ中
    Serial.println(playing);

    //delay(100);
}

void play(unsigned char Track)
{
    unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3};
    mySerial.write(play, 6);
}
// void spPlay(unsigned char Tr)
// {
//     unsigned char spPlay[6] = {0xAA, 0x07, 0x02, 0x00, Tr, Tr + 0xB3};
//     mySerial.write(spPlay, 6);
// }
void volume(unsigned char vol)
{
    unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
    mySerial.write(volume, 5);
}

// void stop(unsigned char stop)
// {
//     unsigned char stop[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
// }