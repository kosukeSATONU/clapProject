/** @file       ApplauseControl.ino
 *  @brief      拍手制御
 *
 *  @par        ProductName
 *
 *  @author
 *  @version    0.1
 *  @date       2021/11/28
 *  @attention  特になし
 *  @par        Revision
 *  $Id$
 *  @par        Copyright
 *
 *  @par        History
 *              version 0.0.1    (2021/11/28) : Initial version
 *
 *  *  サンプルコード等を記述
 *  @code
 *  @endcode
 *
 */
/* Driver Includes ------------------------------------------------------------*/

/* Application Includes -------------------------------------------------------*/
#include <SoftwareSerial.h>

#include "DFRobotDFPlayerMini.h"

/* Extern Variables -----------------------------------------------------------*/

/* Private Definitions --------------------------------------------------------*/
#define CONSOLE Serial
#define SKETCH_NAME "Applause Control"
#define VERSION "0.1"

#define _DEBUG_TEST_ false ///< デバッグテスト

#define USE_WAKEUP_SOUND true   ///< 起動音を鳴らす場合はtrue ファイルNo=1、鳴らさない場合はfalse
#define CONSOLE_BAUDRATE (9600) ///< デバッグモニタ用のボーレート

#define USE_DFPLAYER_MINI true ///< DFRobotDFPlayerMini(DFR0299)を使用するかどうか

#define VOICE_MODULE_PIN_RX 11           ///< ボイスモジュール(DFR0299)のRXピン
#define VOICE_MODULE_PIN_TX 10           ///< ボイスモジュール(DFR0299)のTXピン
#define VOICE_MODULE_PIN_BUSY A3         ///< ボイスモジュール(DFR0299)のBUSYピン
#define VOICE_MODULE_BAUDRATE (9600)     ///< ボイスモジュール(DFR0299)のボーレート
#define VOICE_MODULE_SPEED_DEFAULT (250) ///< ボイスモジュールの再生速度(ディレイ時間)のデフォルト値(TODO:暫定)
#define VOICE_MODULE_SPEED_MIN (500)     ///< ボイスモジュールの再生速度(ディレイ時間)の最小値(TODO:暫定)
#define VOICE_MODULE_SPEED_MAX (10)      ///< ボイスモジュールの再生速度(ディレイ時間)の最大値(TODO:暫定)
#define VOICE_MODULE_VOLUME_DEFAULT (5)  ///< ボイスモジュールの音量のデフォルト値(TODO:暫定)
#define VOICE_MODULE_VOLUME_MIN (5)      ///< ボイスモジュールの音量の最小値
#define VOICE_MODULE_VOLUME_MAX (30)     ///< ボイスモジュールの音量の最大値

#define JOYSTICK_PIN_X A1 ///< ジョイステックX軸ピン
#define JOYSTICK_PIN_Y A0 ///< ジョイステックY軸ピン
#define JOYSTICK_PIN_SW 2 ///< ジョイステックセンタースイッチピン

#define JOYSTICK_CENTER_POS_X (326.0f) ///< X軸方向の中心位置の値(アナログ値[0～1024])(TODO:暫定)
#define JOYSTICK_CENTER_POS_Y (326.0f) ///< Y軸方向の中心位置の値(アナログ値[0～1024])(TODO:暫定)
#define JOYSTICK_POS_X_MIN (0)         ///< X軸方向を-方向に倒したときの最小値(アナログ値[0～1024])(TODO:暫定)
#define JOYSTICK_POS_X_MAX (664)       ///< X軸方向を+方向に倒したときの最大値(アナログ値[0～1024])(TODO:暫定)
#define JOYSTICK_POS_Y_MIN (0)         ///< Y軸方向を-方向に倒したときの最小値(アナログ値[0～1024])(TODO:暫定)
#define JOYSTICK_POS_Y_MAX (666)       ///< Y軸方向を+方向に倒したときの最大値(アナログ値[0～1024])(TODO:暫定)

#define JOYSTICK_RAPID_REACTION_THRESHOL (250) ///< ジョイスティックを急激に倒されたかどうかを判定する閾値(TODO:暫定) \
                                               ///< 前回との差分を取った操作量との比較(急激に倒せば操作量が大きくなる)

#define INTERVAL_MS (100) ///< インターバル時間[ms](TODO:暫定)

#define INVALID_AREA_THRESHOLD (0.05f) ///< ジョイスティックの無効領域の閾値(%で設定:デフォルトは5%とする)(TODO:暫定) \
                                       ///< 中心から閾値以上変化があれば有効データとみなす。
const float THRESHOLD_X = JOYSTICK_CENTER_POS_X * INVALID_AREA_THRESHOLD;
const float THRESHOLD_Y = JOYSTICK_CENTER_POS_Y * INVALID_AREA_THRESHOLD;

/* Private Typedef ------------------------------------------------------------*/
/*
 * ジョイステックの構造体
 */
typedef struct
{
    bool OnOff;     ///< 中心のボタンが押されたかどうか
    int16_t ValueX; ///< X方向の読み取り値
    int16_t ValueY; ///< Y方向の読み取り値
    bool IsValidX;  ///< X軸方向の値が有効かどうか
    bool IsValidY;  ///< Y軸方向の値が有効かどうか
} JoystickInfotypeDef;

/*
 * サウンドプレイヤーの構造体
 */
typedef struct
{
    uint8_t Mode;        ///< 再生モード(0=無効,1=拍手,2=大歓声)
    uint8_t Volume;      ///< 音量(0-30)
    uint16_t Speed;      ///< 再生速度
    bool IsStart;        ///< 再生開始
    bool IsStop;         ///< 再生停止
    bool IsBusy;         ///< Busy
    uint16_t BusyAnalog; ///< Busyピンのアナログ値
} PlayerInfotypeDef;

/* Private Macro --------------------------------------------------------------*/
/* Private Variables ----------------------------------------------------------*/
JoystickInfotypeDef JoystickInfo = {0};     ///< ジョイステックの情報
JoystickInfotypeDef JoystickInfoLast = {0}; ///< 前回のジョイステックの情報
PlayerInfotypeDef PlayerInfo = {0};         ///< サウンドプレイヤーの情報

SoftwareSerial mySoftwareSerial(VOICE_MODULE_PIN_RX, VOICE_MODULE_PIN_TX); ///< ボイスモジュールのUART
DFRobotDFPlayerMini myDFPlayer;                                            ///< ボイスモジュールのAPI

/* Private Function Prototypes ------------------------------------------------*/

/* ------------------------------------------------------------------------------
  P R O G R A M  C O D E
 ------------------------------------------------------------------------------*/

//*******************************************************************************
/**
 * @brief       初期化
 *
 * @param       無し
 * @return      無し
 */
//*******************************************************************************
void setup()
{
    delay(1000);

    CONSOLE.begin(CONSOLE_BAUDRATE);

    CONSOLE.println();
    CONSOLE.print(F("Welcome to "));
    CONSOLE.print(SKETCH_NAME);
    CONSOLE.print(F(" "));
    CONSOLE.println(VERSION);

    // ジョイステックの設定
    pinMode(JOYSTICK_PIN_X, INPUT);
    pinMode(JOYSTICK_PIN_Y, INPUT);
    pinMode(JOYSTICK_PIN_SW, INPUT_PULLUP); // アクティブLow

    uint8_t volume = 0;
    CONSOLE.println(F("[Volume Test]"));

    int16_t testDataVolume[] = {JOYSTICK_POS_Y_MIN, JOYSTICK_POS_Y_MAX, JOYSTICK_CENTER_POS_Y, 100, 200, 300, 400, 500, 600};
    int count = sizeof(testDataVolume) / sizeof(int16_t);
    for (int i = 0; i < count; i++)
    {
        JoystickInfo.ValueY = testDataVolume[i];
        PlayerInfo.Volume = convertToVolume(JoystickInfo.ValueY);
        CONSOLE.print(F("ValueY:"));
        CONSOLE.print(JoystickInfo.ValueY);
        CONSOLE.print(F(",Volume:"));
        CONSOLE.println(PlayerInfo.Volume);
    }
    CONSOLE.println("");

    CONSOLE.println(F("[Speed Test]"));
    int16_t testDataSpeed[] = {JOYSTICK_POS_X_MIN, JOYSTICK_POS_X_MAX, JOYSTICK_CENTER_POS_X, 100, 200, 300, 400, 500, 600};
    count = sizeof(testDataSpeed) / sizeof(int16_t);
    for (int i = 0; i < count; i++)
    {
        JoystickInfo.ValueX = testDataSpeed[i];
        PlayerInfo.Speed = convertToSpeed(JoystickInfo.ValueX);
        CONSOLE.print(F("ValueX:"));
        CONSOLE.print(JoystickInfo.ValueX);
        CONSOLE.print(F(",Speed:"));
        CONSOLE.println(PlayerInfo.Speed);
    }
    CONSOLE.println("");

    // プレーヤーの設定
#if USE_DFPLAYER_MINI
    // BUSYピン
    pinMode(VOICE_MODULE_PIN_BUSY, INPUT);

    mySoftwareSerial.begin(VOICE_MODULE_BAUDRATE);
    if (!myDFPlayer.begin(mySoftwareSerial)) //Use softwareSerial to communicate with mp3.
    {
        CONSOLE.println(F("Unable to begin:"));
        CONSOLE.println(F("1.Please recheck the connection!"));
        CONSOLE.println(F("2.Please insert the SD card!"));
        while (true)
            ;
    }
    CONSOLE.println(F("DFPlayer Mini online."));

    myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

    // 起動音を鳴らす(必要あれば)
#ifdef USE_WAKEUP_SOUND
    myDFPlayer.volume(5); // ボリューム設定 0 to 30
    myDFPlayer.play(1);   // 起動音現No1(TODO:仮)
    CONSOLE.println("WakeUp Sound:play=1, volum=5");
    delay(3000);
#endif
#endif // USE_DFPLAYER_MINI

    CONSOLE.println("Initializing... Done");
}

//*******************************************************************************
/**
 * @brief       メインループ処理
 *
 * @param       無し
 * @return      無し
 */
//*******************************************************************************
void loop()
{
    unsigned long timeTick_ms = millis();
    int16_t diff_x = 0;
    int16_t diff_y = 0;

    // ジョイスティックのポジション取得
    JoystickInfo.ValueX = analogRead(JOYSTICK_PIN_X);
    JoystickInfo.ValueY = analogRead(JOYSTICK_PIN_Y);
    JoystickInfo.OnOff = digitalRead(JOYSTICK_PIN_SW);
    // Busyピンの判定
    PlayerInfo.BusyAnalog = analogRead(VOICE_MODULE_PIN_BUSY);
    PlayerInfo.IsBusy = (PlayerInfo.BusyAnalog < 70) ? true : false; //音再生時450~455くらい

    // ジョイスティックの有効域の判定
    // X軸:再生速度
    if ((JoystickInfo.ValueX > (JOYSTICK_CENTER_POS_X - THRESHOLD_X)) && (JoystickInfo.ValueX < (JOYSTICK_CENTER_POS_X + THRESHOLD_X)))
    {
        // 無効
        JoystickInfo.IsValidX = false;
    }
    else
    {
        // 有効
        JoystickInfo.IsValidX = true;
        if (PlayerInfo.Mode < 2) // 大歓声を検知していると停止する迄更新しない
        {
            PlayerInfo.Mode = 1; // 拍手モード
        }

        // 変化量をチェックする(勢いよく倒したかどうか)
        // 操作量 = 今回値 ― 前回値
        diff_x = JoystickInfo.ValueX - JoystickInfoLast.ValueX;
        if (JoystickInfo.ValueX > JOYSTICK_CENTER_POS_X)
        {
            // ＋方向
            if (JOYSTICK_RAPID_REACTION_THRESHOL < diff_x)
            {
                // 急操作検知
                PlayerInfo.Mode = 2; // 大歓声モード
            }
        }
        else
        {
            // ―方向
            if (-1 * JOYSTICK_RAPID_REACTION_THRESHOL > diff_x)
            {
                // 急操作検知
                PlayerInfo.Mode = 2; // 大歓声モード
            }
        }
    }

    // Y軸:音量
    if ((JoystickInfo.ValueY > (JOYSTICK_CENTER_POS_Y - THRESHOLD_Y)) && (JoystickInfo.ValueY < (JOYSTICK_CENTER_POS_Y + THRESHOLD_Y)))
    {
        // 無効
        JoystickInfo.IsValidY = false;
    }
    else
    {
        // 有効
        JoystickInfo.IsValidY = true;
        if (PlayerInfo.Mode < 2) // 大歓声を検知していると停止する迄更新しない
        {
            PlayerInfo.Mode = 1; // 拍手モード
        }

        // 変化量をチェックする(勢いよく倒したかどうか)
        // 操作量 = 今回値 ― 前回値
        diff_y = JoystickInfo.ValueY - JoystickInfoLast.ValueY;
        if (JoystickInfo.ValueY > JOYSTICK_CENTER_POS_Y)
        {
            // ＋方向
            if (JOYSTICK_RAPID_REACTION_THRESHOL < diff_y)
            {
                // 急操作検知
                PlayerInfo.Mode = 2; // 大歓声モード
            }
        }
        else
        {
            // ―方向
            if (-1 * JOYSTICK_RAPID_REACTION_THRESHOL > diff_y)
            {
                // 急操作検知
                PlayerInfo.Mode = 2; // 大歓声モード
            }
        }
    }

    // X軸の操作量を再生速度(ディレイ時間)の値に変換
    if (JoystickInfo.IsValidX)
    {
        // 操作量を再生速度に変換
        PlayerInfo.Speed = convertToSpeed(JoystickInfo.ValueX);
    }
    else
    {
        // デフォルト
        PlayerInfo.Speed = VOICE_MODULE_SPEED_DEFAULT;
    }

    // Y軸の操作量を音量(0 to 30)の値に変換
    if (JoystickInfo.IsValidY)
    {
        // 操作量を音量に変換
        PlayerInfo.Volume = convertToVolume(JoystickInfo.ValueY);
    }
    else
    {
        // デフォルト
        PlayerInfo.Volume = VOICE_MODULE_VOLUME_DEFAULT;
    }

    // 再生判定
    if (JoystickInfo.IsValidX == false && JoystickInfo.IsValidY == false)
    {
        // 停止
        if (PlayerInfo.IsBusy == false) // 再生が終わるのを待つ
        {
            // myDFPlayer.stop();
            PlayerInfo.IsStart = false;
            PlayerInfo.Mode = 0;
        }
    }
    else
    {
        // 再生
        if (PlayerInfo.IsBusy == false) // 再生が終わっていれば再生
        {
            PlayerInfo.IsStart = true;
        }
        else
        {
            // 再生中の音量変更
            myDFPlayer.volume(PlayerInfo.Volume); // ボリューム設定 0 to 30
        }
    }

    CONSOLE.print("[");
    CONSOLE.print(timeTick_ms);
    CONSOLE.print("]\t");
    CONSOLE.print("Joystick\t");
    CONSOLE.print(",X:");
    CONSOLE.print(JoystickInfo.ValueX);
    CONSOLE.print(",X Last:");
    CONSOLE.print(JoystickInfoLast.ValueX);
    CONSOLE.print(",Y:");
    CONSOLE.print(JoystickInfo.ValueY);
    CONSOLE.print(",Y Last:");
    CONSOLE.print(JoystickInfoLast.ValueY);
    CONSOLE.print(",IsValidX:");
    CONSOLE.print(JoystickInfo.IsValidX);
    CONSOLE.print(",IsValidX:");
    CONSOLE.print(JoystickInfo.IsValidX);
    CONSOLE.print(",SW:");
    CONSOLE.print(JoystickInfo.OnOff);
    CONSOLE.print(",SW Last:");
    CONSOLE.print(JoystickInfoLast.OnOff);
    CONSOLE.print(",Diff X:");
    CONSOLE.print(diff_x);
    CONSOLE.print(",Diff Y:");
    CONSOLE.print(diff_y);
    CONSOLE.print("\t");
    CONSOLE.print("Sound Play\t");
    CONSOLE.print(",Mode:");
    CONSOLE.print(PlayerInfo.Mode);
    CONSOLE.print(",Speed:");
    CONSOLE.print(PlayerInfo.Speed);
    CONSOLE.print(",Volume:");
    CONSOLE.print(PlayerInfo.Volume);
    CONSOLE.print(",IsStart:");
    CONSOLE.print(PlayerInfo.IsStart);
    CONSOLE.print(",IsStop:");
    CONSOLE.print(PlayerInfo.IsStop);
    CONSOLE.print(",IsBusy:");
    CONSOLE.print(PlayerInfo.IsBusy);
    CONSOLE.print(",BusyAnalog:");
    CONSOLE.print(PlayerInfo.BusyAnalog);
    CONSOLE.println("");

    // 再生されていなければ再生
    if (PlayerInfo.IsStart && (PlayerInfo.IsBusy == false))
    {
        switch (PlayerInfo.Mode)
        {
        case 1: // 拍手
            CONSOLE.print("[");
            CONSOLE.print(timeTick_ms);
            CONSOLE.print("]\t");
            CONSOLE.println("playTask1:Start");

            playTask1();

            CONSOLE.print("[");
            CONSOLE.print(timeTick_ms);
            CONSOLE.print("]\t");
            CONSOLE.println("playTask1:Stop");
            break;

        case 2: // 大歓声
            CONSOLE.print("[");
            CONSOLE.print(timeTick_ms);
            CONSOLE.print("]\t");
            CONSOLE.println("playTask2:Start");

            playTask2();

            CONSOLE.print("[");
            CONSOLE.print(timeTick_ms);
            CONSOLE.print("]\t");
            CONSOLE.println("playTask2:Stop");
            break;

        default:
            break;
        }
    }

    if (myDFPlayer.available())
    {
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }

    // 最後の値を保持
    JoystickInfoLast = JoystickInfo;

    // インターバル
    delay(INTERVAL_MS);
}

//*******************************************************************************
/**
 * @brief       操作量 -> 再生速度処理
 *
 * @param       int16_t value
 * @return      再生速度(ディレイ時間)
 */
//*******************************************************************************
uint16_t convertToSpeed(int16_t value)
{
    long min = 0;
    long max = 0;
    long result = 0;

    if (JOYSTICK_CENTER_POS_X < value)
    {
        min = (JOYSTICK_CENTER_POS_X + THRESHOLD_X);
        max = JOYSTICK_POS_X_MAX;
    }
    else
    {
        min = (JOYSTICK_CENTER_POS_X + THRESHOLD_X);
        max = JOYSTICK_POS_X_MIN;
    }
    // スケール変換
    result = map(value, min, max, VOICE_MODULE_SPEED_MIN, VOICE_MODULE_SPEED_MAX);

    // 最大/最小 ガード
    if (result < VOICE_MODULE_SPEED_MAX)
    {
        result = VOICE_MODULE_SPEED_MAX;
    }
    if (result > VOICE_MODULE_SPEED_MIN)
    {
        result = VOICE_MODULE_SPEED_MIN;
    }

    return (uint16_t)result;
}

//*******************************************************************************
/**
 * @brief       操作量 -> 音量変換処理
 *
 * @param       int16_t value
 * @return      音量(0-30)
 */
//*******************************************************************************
uint8_t convertToVolume(int16_t value)
{
    long min = 0;
    long max = 0;
    long result = 0;

    if (JOYSTICK_CENTER_POS_Y < value)
    {
        min = (JOYSTICK_CENTER_POS_Y + THRESHOLD_Y);
        max = JOYSTICK_POS_Y_MAX;
    }
    else
    {
        min = (JOYSTICK_CENTER_POS_Y + THRESHOLD_Y);
        max = JOYSTICK_POS_Y_MIN;
    }
    // スケール変換
    result = map(value, min, max, VOICE_MODULE_VOLUME_MIN, VOICE_MODULE_VOLUME_MAX);

    // 最大/最小 ガード
    if (result > VOICE_MODULE_VOLUME_MAX)
    {
        result = VOICE_MODULE_VOLUME_MAX;
    }
    if (result < VOICE_MODULE_VOLUME_MIN)
    {
        result = VOICE_MODULE_VOLUME_MIN;
    }

    return (uint8_t)result;
}

//*******************************************************************************
/**
 * @brief       再生タスク処理1(拍手)
 *
 * @param       無し
 * @return      無し
 */
//*******************************************************************************
void playTask1(void)
{
#if USE_DFPLAYER_MINI

    // TODO:ここに拍手を実現するための処理を実装する
    myDFPlayer.volume(PlayerInfo.Volume); // ボリューム設定 0 to 30
#if _DEBUG_TEST_
    // デバッグテスト
    myDFPlayer.play(2); // TODO:拍手音声
    delay(PlayerInfo.Speed);
#else
    for (int i = 0; i < 5; i++) // TODO:繰り返し回数は調整ください
    {
        myDFPlayer.play(1); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(2); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(3); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(4); // TODO:拍手音声
        delay(PlayerInfo.Speed);
    }
#endif

#endif // USE_DFPLAYER_MINI
}

//*******************************************************************************
/**
 * @brief       再生タスク処理2(大歓声)
 *
 * @param       無し
 * @return      無し
 */
//*******************************************************************************
void playTask2(void)
{
#if USE_DFPLAYER_MINI

    // TODO:ここに大歓声を実現するための処理を実装する
    myDFPlayer.volume(PlayerInfo.Volume); // ボリューム設定 0 to 30
#if _DEBUG_TEST_
    // デバッグテスト
    myDFPlayer.play(10); // TODO:歓声音声
#else
    myDFPlayer.play(10);        // TODO:歓声音声
    for (int i = 0; i < 5; i++) // TODO:繰り返し回数は調整ください
    {
        myDFPlayer.play(1); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(2); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(3); // TODO:拍手音声
        delay(PlayerInfo.Speed);
        myDFPlayer.play(4); // TODO:拍手音声
        delay(PlayerInfo.Speed);
    }
#endif

#endif // USE_DFPLAYER_MINI
}

void printDetail(uint8_t type, int value)
{
    switch (type)
    {
    case TimeOut:
        Serial.println(F("Time Out!"));
        break;
    case WrongStack:
        Serial.println(F("Stack Wrong!"));
        break;
    case DFPlayerCardInserted:
        Serial.println(F("Card Inserted!"));
        break;
    case DFPlayerCardRemoved:
        Serial.println(F("Card Removed!"));
        break;
    case DFPlayerCardOnline:
        Serial.println(F("Card Online!"));
        break;
    case DFPlayerPlayFinished:
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    case DFPlayerError:
        Serial.print(F("DFPlayerError:"));
        switch (value)
        {
        case Busy:
            Serial.println(F("Card not found"));
            break;
        case Sleeping:
            Serial.println(F("Sleeping"));
            break;
        case SerialWrongStack:
            Serial.println(F("Get Wrong Stack"));
            break;
        case CheckSumNotMatch:
            Serial.println(F("Check Sum Not Match"));
            break;
        case FileIndexOut:
            Serial.println(F("File Index Out of Bound"));
            break;
        case FileMismatch:
            Serial.println(F("Cannot Find File"));
            break;
        case Advertise:
            Serial.println(F("In Advertise"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}