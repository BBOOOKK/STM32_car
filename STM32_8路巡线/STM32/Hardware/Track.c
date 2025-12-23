#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Track.h"

/*
 * TrackN: 8-bit mask of "line detected".
 * Bit7 = leftmost sensor (Track_S0), Bit0 = rightmost sensor (Track_S7)
 */
uint8_t TrackN = 0;
static float Err = 0.0f;

//读取单个传感器是否压到线
static uint8_t Track_IsLine(GPIO_TypeDef* port, uint16_t pin)
{
#if (TRACK_ACTIVE_LOW)
    /* Active-low sensor output: 0 means on-line */
    return (uint8_t)(!GPIO_ReadInputDataBit(port, pin));
#else
    /* Active-high sensor output: 1 means on-line */
    return (uint8_t)(GPIO_ReadInputDataBit(port, pin));
#endif
}

void Track_Init(void)
{
    /* Enable GPIO clock for the sensor port */
    RCC_APB2PeriphClockCmd(TRACK_RCC, ENABLE);

    GPIO_InitTypeDef GPIOStructure;
    GPIOStructure.GPIO_Mode = GPIO_Mode_IPD; /* keep pull-down as your original */
    GPIOStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOStructure.GPIO_Pin = Track_S0 | Track_S1 | Track_S2 | Track_S3 |
                             Track_S4 | Track_S5 | Track_S6 | Track_S7;
    GPIO_Init(TRACK_PORT, &GPIOStructure);
}

void Read_Track_DATA(uint8_t* arr)
{
    uint8_t bits = 0;

    //读取各路传感器的压线状态，并将状态左移n位
    /* Pack as: Bit7..Bit0 = S0..S7 (left -> right) */
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S0) << 7);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S1) << 6);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S2) << 5);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S3) << 4);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S4) << 3);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S5) << 2);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S6) << 1);
    bits |= (uint8_t)(Track_IsLine(TRACK_PORT, Track_S7) << 0);

    TrackN = bits;
    if (arr != 0) {
        *arr = bits;
    }
}

float Track_Err(uint16_t car_state)
{
    /* Symmetric weights for 8 sensors (left -> right) */
    static const int8_t w[8] = { -7, -5, -3, -1, 1, 3, 5, 7 };

    int sum = 0;
    int cnt = 0;

    /* TrackN bit7..bit0 correspond to w[0]..w[7] */
    int i;
    for (i = 0; i < 8; i++) {
        uint8_t b = (uint8_t)((TrackN >> (7 - i)) & 0x01);
        if (b) {
            sum += w[i];
            cnt++;
        }
    }

    /* If line is lost (cnt == 0), keep the last Err */
    if (cnt != 0) {
        Err = (float)sum / (float)cnt;
    }

    /* Optional one-sided limit based on car_state (kept from your original code) */
    if (car_state == 0x1012 || car_state == 0x1013) {
        if (Err <= 0) Err = 0;
    } else if (car_state == 0x1022 || car_state == 0x1023) {
        if (Err >= 0) Err = 0;
    }

    return Err;
}

