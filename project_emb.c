#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <softPwm.h>
#include <unistd.h>

// 디스플레이 I2C 주소 (주소 확인 필요. 일반적으로 0x27 또는 0x3F)
#define LCD_ADDR 0x27

// LCD 명령
#define LCD_CHR 1 // 문자 모드
#define LCD_CMD 0 // 명령 모드

// 비트 플래그
#define LCD_BACKLIGHT 0x08 // 백라이트 ON
#define ENABLE 0x04        // Enable 비트

#define MAX_TIMINGS 85
#define DHT_PIN 20
#define RED_PIN 13
#define YEL_PIN 19
#define GRN_PIN 26
#define MOTOR_PIN 16

int data[5] = {0, 0, 0, 0, 0};

void lcdToggleEnable(int fd, int bits)
{
    wiringPiI2CWrite(fd, bits | ENABLE);
    usleep(500); // 500μs
    wiringPiI2CWrite(fd, bits & ~ENABLE);
    usleep(500); // 500μs
}

void lcdSendByte(int fd, int bits, int mode)
{
    int highBits = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    int lowBits = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CWrite(fd, highBits);
    lcdToggleEnable(fd, highBits);

    wiringPiI2CWrite(fd, lowBits);
    lcdToggleEnable(fd, lowBits);
}

void lcdInit(int fd)
{
    lcdSendByte(fd, 0x33, LCD_CMD); // 초기화
    lcdSendByte(fd, 0x32, LCD_CMD); // 초기화
    lcdSendByte(fd, 0x06, LCD_CMD); // 커서 이동 방향
    lcdSendByte(fd, 0x0C, LCD_CMD); // 화면 켜기
    lcdSendByte(fd, 0x28, LCD_CMD); // 2라인 모드
    lcdSendByte(fd, 0x01, LCD_CMD); // 화면 지우기
    usleep(5000);                   // 지우기 대기
}

void lcdSetCursor(int fd, int line)
{
    int address;
    if (line == 0)
    {
        address = 0x80;
    }
    else
    {
        address = 0xC0;
    }
    lcdSendByte(fd, address, LCD_CMD);
}

void lcdPrint(int fd, const char *message)
{
    while (*message)
    {
        lcdSendByte(fd, *message++, LCD_CHR);
    }
}

int servoControl_5()
{
    softPwmCreate(MOTOR_PIN, 0, 200);
    softPwmWrite(MOTOR_PIN, 5);
    delay(600);

    return 0;
}

int servoControl_25()
{
    softPwmCreate(MOTOR_PIN, 0, 200);
    softPwmWrite(MOTOR_PIN, 25);
    delay(600);

    return 0;
}

void ledOn(int pin)
{
    digitalWrite(pin, HIGH);
}

void ledOff(int pin)
{
    digitalWrite(pin, LOW);
}

void read_dht_data(int lcd)
{
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, LOW);
    delay(18);

    pinMode(DHT_PIN, INPUT);

    for (i = 0; i < MAX_TIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHT_PIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
            {
                break;
            }
        }
        laststate = digitalRead(DHT_PIN);

        if (counter == 255)
            break;

        if ((i >= 4) && (i % 2 == 0))
        {

            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
    {
        printf("Temperature = %d.%d C\n", data[2], data[3]);
        if (data[2] < 21)
        {
            ledOn(GRN_PIN);
            ledOff(RED_PIN);
            ledOff(YEL_PIN);
            lcdSetCursor(lcd, 0);
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "Temp: %dC", data[2]);
            lcdPrint(lcd, buffer);
            lcdSetCursor(lcd, 1);
            lcdPrint(lcd, "Safety Zone");
            servoControl_5();
        }
        else if (data[2] < 22)
        {
            ledOn(YEL_PIN);
            ledOff(GRN_PIN);
            ledOff(RED_PIN);
            lcdSetCursor(lcd, 0);
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "Temp: %dC", data[2]);
            lcdPrint(lcd, buffer);
            lcdSetCursor(lcd, 1);
            lcdPrint(lcd, "Be careful!! Temp is high");
            servoControl_25();
        }
        else
        {
            ledOn(RED_PIN);
            ledOff(YEL_PIN);
            ledOff(GRN_PIN);
            lcdSetCursor(lcd, 0);
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "Temp: %dC", data[2]);
            lcdPrint(lcd, buffer);
            lcdSetCursor(lcd, 1);
            lcdPrint(lcd, "Dangerous!! Take shelter");
            servoControl_25();
        }
    }
    else
    {
        printf("--------------------\n");
    }
}

int main(void)
{
    int lcd;

    if (wiringPiSetupGpio() == -1)
        exit(1);

    lcd = wiringPiI2CSetup(LCD_ADDR);
    if (lcd == -1)
    {
        fprintf(stderr, "I2C 초기화 실패!\n");
        return EXIT_FAILURE;
    }

    // LCD 초기화
    lcdInit(lcd);

    pinMode(RED_PIN, OUTPUT);
    pinMode(YEL_PIN, OUTPUT);
    pinMode(GRN_PIN, OUTPUT);

    while (1)
    {
        read_dht_data(lcd);
        delay(2000);
    }

    return (0);
}
