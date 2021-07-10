#include <SoftwareSerial.h>
//http://tomorrow.meyon.gonna.jp/?eid=1005431
//参考

SoftwareSerial mySerial(10, 11); //TX&RX for MP3VoiceModule

int LED_pin = 13;
int SW_pin = 2;
int playing_pin = A3; //busypin

uint16_t count_low = 0;

unsigned char order[4] = {0xAA, 0x02, 0x00, 0xB0};
int playWait = 100;
int checkSW = 0;
int changeNum = 0;
int playing = 0;
int buttonState = 0;

void setup()
{
    pinMode(SW_pin, INPUT);
    pinMode(LED_pin, OUTPUT);    //スイッチLED用セットアップ
    pinMode(playing_pin, INPUT); //Acquire BUSYpin 演奏時HIGH

    Serial.begin(9600);
    mySerial.begin(9600);
    volume(0x10); //Volume settings 0x00~0x1E//ボリュームコントロール
}

void loop()
{

    checkSW = digitalRead(SW_pin);
    playing = analogRead(playing_pin); //450~455くらい

    //入力待ち スイッチ押されるまでなにもしない
    Serial.print("  checkSW=");
    Serial.print(checkSW);
    if (checkSW == 1)
    {
        digitalWrite(LED_pin, 1);
        buttonState = 1;
        changeNum = 0;
    }

    //変化を調べる 入力押されたら処理スタート
    Serial.print("  buttonState=");
    Serial.print(buttonState);

    if (buttonState == 1)
    {
        for (int i = 0; i < 10; i++) //500m/sスイッチの状態変化をみるon→off/off→on
        {
            delay(5);
            checkSW = digitalRead(SW_pin);
            if (checkSW != buttonState) //スイッチ状態が変化していたら
            {
                changeNum++;
                if (changeNum >= 1) //状態が変化していたらループを抜ける
                {
                    break;
                }
                buttonState = checkSW; //buttonStateを現在のボタンの状態にする
            }
        }

        while (checkSW == 1) //ボタンがoffになるまで状態を読み続けて待つ
        {
            checkSW = digitalRead(SW_pin);
            changeNum = 2;
        }
        buttonState = 0;
    }

    //変化に合わせて音楽再生
    switch (changeNum)
    {
    case 0: // changeNumが0のとき実行されるホールド

        changeNum = 0;
        break;

    case 1:
        if (playing == 0)
        {
            play(2);                  //play(トラック番号) 1=長い拍手/2=単拍手
            mySerial.write(order, 4); //order play
            //delay(200);
            changeNum = 0;
        } // changeNumが1のとき実行されるシングル
        break;

    case 2:                       //changeNumが2のとき実行される
        play(1);                  //play(トラック番号) 1=長い拍手/2=単拍手
        mySerial.write(order, 4); //order play
        changeNum = 0;
        break;
    }
    //モニター

    Serial.print("  changeNum=");
    Serial.print(changeNum);
    Serial.print("  playing=");
    Serial.println(playing);
    //

    //delay(100);
}

//mp3モジュール・playの設定
void play(unsigned char Track)
{
    unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3};
    mySerial.write(play, 6);
}

/*
 void spPlay(unsigned char Tr)
 {
     unsigned char spPlay[6] = {0xAA, 0x07, 0x02, 0x00, Tr, Tr + 0xB3};
     mySerial.write(spPlay, 6);
 }
*/

void volume(unsigned char vol)
{
    unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
    mySerial.write(volume, 5);
}

// void stop(unsigned char stop)
// {
//     unsigned char stop[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
// }