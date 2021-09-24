#include <SoftwareSerial.h>

/* for アナログスティック

//参考 http://tomorrow.meyon.gonna.jp/?eid=1005431
//ノイズ対策！3.3V製品なので、arduino3.3Vから電源を取るか、5Vに1kΩ抵抗挟むこと
//5Vの時はボリュームモジュールのVCCに1kΩ挟むこと
//play(トラック番号) 1=無音/2=高音単/3=ロング35秒/4=低音単/=多分書き込んだ順番に番号がが割り振られている！

*/

SoftwareSerial mySerial(10, 11); //TX&RX for MP3VoiceModule

int LED_pin = 13;
int SW_pin = 2;
int playing_pin = A3; //busypin

//拍手タクトスイッチ
unsigned char order[4] = {0xAA, 0x02, 0x00, 0xB0};
int playWait = 100;
int checkSW = 0;
int changeNum = 0;
int playing = 0;
int buttonState = 0;

//アナログスティック
int stSW_pin = 2; //中心のタクトスイッチ=押された時LOW→INPUT_PULLUP
int stY_pin = A0;
int stX_pin = A1;
int switch_state = 0;
int x_pos = 0;
int y_pos = 0;

void setup()
{
    //タクトスイッチ用セットアップ
    pinMode(SW_pin, INPUT);
    pinMode(LED_pin, OUTPUT);    //スイッチLED用セットアップ
    pinMode(playing_pin, INPUT); //Acquire BUSYpin 演奏時HIGH

    //アナログスティック用セットアップ
    pinMode(stX_pin, INPUT);
    pinMode(stY_pin, INPUT);
    pinMode(stSW_pin, INPUT_PULLUP); //

    Serial.begin(9600);
    mySaerial.begin(9600);
    volume(0x1E); //Volume settings 0x00~0x1E//ボリュームコントロール
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

            play(3);                  //play(トラック番号) 1=無音/2=高音単/3=ロング35秒/4=低音単/
            mySerial.write(order, 4); //order play
            //delay(20);
            while (checkSW == 1)
            {
                checkSW = digitalRead(SW_pin);
                delay(5);
            }
            changeNum = 2;
        }
        buttonState = 0;
    }
    Serial.print("  changeNum=");
    Serial.print(changeNum);

    //変化に合わせて音楽再生
    switch (changeNum)
    {
    case 0: // changeNumが0のとき実行されるホールド

        changeNum = 0;
        break;

    case 1:
        if (playing == 0)
        {
            play(2);                  //play(トラック番号) 1=無音/2=高音単/3=ロング35秒/4=低音単/
            mySerial.write(order, 4); //order play
            //delay(100);
            changeNum = 0;
        } // changeNumが1のとき実行されるシングル
        break;

    case 2:                       //changeNumが2のとき実行される
        play(1);                  //play(トラック番号) 1=無音/2=高音単/3=ロング35秒/4=低音単/
        mySerial.write(order, 4); //order play
        //delay(100);
        changeNum = 0;
        break;
    }
    //モニター

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

// void DFRVM_Stop()
// {
//      char dt[4] = {0xAA,0x04,0x00,0x00} ;
//      mySerial.write(dt,4) ;  // [04]コマンドの送信
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