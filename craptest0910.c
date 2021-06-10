craptest0910.c

#include <SoftwareSerial.h>
#include <Wire.h>

    //uint8_t DEVICE_ADDRESS = 0x1D; //If SDO=L, 0x53これなんだろう

    SoftwareSerial Serial1(3, 2); //TX&RX for MP3VoiceModule

int th = 14;         //Threshold for tappingタッピングの閾値
int tiltth = 170;    // Threshold for tilt傾き閾値
int playwait = 2000; //Waiting time after sound

unsigned char order[4] = {0xAA, 0x06, 0x00, 0xB0};
int tapnum = 0; //タップ数を格納するための変数の型を作っている。
int tilthist = 0;
int tapped = 0;
uint8_t RegTbl[6];

void setup()
{
    //Serial.begin(9600);

    //MP3VoiceModule
    Serial1.begin(9600);
    volume(0x19); //Volume settings 0x00-0x1E
}

void loop()
{
    //Aqcuire tapped or tiltedタップされたか、傾きされたかを所得
    if (checktilt() == 1)
    {
        if (tilthist == 1)
        {
            tapnum = 0;
            tilthist = 1;
        }
        else
        {
            tapnum = -1;
            tilthist = 1;
        }
    }
    else if (abs(getacc()) > th)
    {
        checksingletap();
        tilthist = 0;
    }
    else if (checktilt() == 0)
    {
        tilthist = 0;
    }

    //Play sound
    if (tapnum == 1 && tapped == 1)
    {
        //Serial.println("tapped1");
        play(0x01); //Play the specified audio:0x01-file0001
        delay(playwait);
    }
    else if (tapnum == 2 && tapped == 1)
    {
        //Serial.println("tapped2");
        play(0x02); //Play the specified audio:0x02-file0002
        tapnum = 0;
        delay(playwait);
    }
    else if (tapnum == -1)
    {
        //Serial.println("tilt");
        play(0x03); //Play the specified audio:0x03-file0003
        delay(playwait);
        tapnum = 0;
    }

    //Reset tap state
    tapped = 0;
    delay(10);
}

int checktilt()
{
    int temptilt1, temptilt2, temptilt3 = 0;
    temptilt1 = abs(getacc());
    delay(10);
    temptilt2 = abs(getacc());
    delay(10);
    temptilt3 = abs(getacc());
    if (temptilt1 + temptilt2 + temptilt3 > 3 * tiltth)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int checktap()
{
    int zacc1, zacc2, zacc3 = 0;
    delay(100);
    zacc1 = abs(getacc());
    delay(10);
    zacc2 = abs(getacc());
    delay(10);
    zacc3 = abs(getacc());

    if (zacc1 + zacc2 + zacc3 < 30)
    {
        tapped = 1;
        return 1;
    }
    else
    {
        tapped = 0;
        return 0;
    }
}

void checksingletap()
{
    if (checktap() == 1)
    {
        tapnum++;
    }
    else
    {
        delay(500);
    }
}

int getacc()
{
    //Move to acc data
    Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x32);
    Wire.endTransmission();

    //Require data
    Wire.requestFrom(DEVICE_ADDRESS, 6);

    //Get 6b data
    int i;
    for (i = 0; i < 6; i++)
    {
        while (Wire.available() == 0)
        {
        }
        RegTbl[i] = Wire.read();
    }

    //Separate axes of x, y, z
    int16_t x = (((int16_t)RegTbl[1]) << 8) | RegTbl[0];
    int16_t y = (((int16_t)RegTbl[3]) << 8) | RegTbl[2];
    int16_t z = (((int16_t)RegTbl[5]) << 8) | RegTbl[4];

    return z + zoffset;
}

void play(unsigned char Track)
{
    unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3};
    Serial1.write(play, 6);
}
void volume(unsigned char vol)
{
    unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE};
    Serial1.write(volume, 5);
}
