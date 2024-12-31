#pragma once

#include <Arduino.h>

#define DISPLAY_MODE 0x00
#define WRITE_REG_AUTO_ADDR 0x40
#define WRITE_REG_FIXED_ADDR 0x44
#define READ_KEY_SCAN 0x42
#define SET_DISPLAY_ADDR 0xC0
#define DISPLAY_OFF 0x80
#define DISPLAY_ON 0x88 | 0x84
#define STB_ENABLE LOW
#define STB_DISABLE HIGH
#define DOT_POSITION 2

uint8_t dioPin = 16;
uint8_t clkPin = 5;
uint8_t stbPin = 4;

const int numberOfDigits = DISPLAY_MODE & 1 ? 5 : 4;
int gridData[numberOfDigits] = {0};

uint16_t num;
uint8_t digit[12] = {
    0b01111110,
    0b00001100,
    0b10110110,
    0b10011110,
    0b11001100,
    0b11011010,
    0b11111010,
    0b01001110,
    0b11111110,
    0b11011110,
    0b00000000, // 10:blank
    0b00100000, // 11:dot};
};

const int blank = 10;
const int dot = 11;

void displayNumbers(int n)
{
    for (int i = 0; i < numberOfDigits; i++)
    {
        int exponentialInDecimal = pow(10, i) + 0.5;
        bool zeroSuppression = (0 != i) && (DOT_POSITION < i) && (exponentialInDecimal > n);
        gridData[i] = zeroSuppression ? blank : n / exponentialInDecimal % 10;
    }
    digitalWrite(stbPin, STB_ENABLE);
    shiftOut(dioPin, clkPin, LSBFIRST, DISPLAY_MODE);
    digitalWrite(stbPin, STB_DISABLE);

    digitalWrite(stbPin, STB_ENABLE);
    shiftOut(dioPin, clkPin, LSBFIRST, WRITE_REG_AUTO_ADDR);
    digitalWrite(stbPin, STB_DISABLE);

    digitalWrite(stbPin, STB_ENABLE);
    shiftOut(dioPin, clkPin, LSBFIRST, SET_DISPLAY_ADDR | 0x00);
    for (int i = 0; i < numberOfDigits; i++)
    {
        shiftOut(dioPin, clkPin, LSBFIRST, digit[gridData[i]]);
        shiftOut(dioPin, clkPin, LSBFIRST, DOT_POSITION == i ? digit[dot] : 0);
    }
    digitalWrite(stbPin, STB_DISABLE);

    digitalWrite(stbPin, STB_ENABLE);
    shiftOut(dioPin, clkPin, LSBFIRST, DISPLAY_ON);
    digitalWrite(stbPin, STB_DISABLE);
}

void tm1630setup()
{
    pinMode(dioPin, OUTPUT);
    pinMode(clkPin, OUTPUT);
    pinMode(stbPin, OUTPUT);
}
