#include <rtc6715.h>

#define SYNTHESIZER_REG_A                           0x00
#define SYNTHESIZER_REG_B                           0x01
#define SYNTHESIZER_REG_C                           0x02
#define SYNTHESIZER_REG_D                           0x03
#define VCO_SWITCH_CAP_CONTROL_REGISTER             0x04
#define DFC_CONTROL_REGISTER                        0x05
#define SIXM_AUDIO_DEMODULATOR_CONTROL_REGISTER     0x06
#define SIXM5_AUDIO_DEMODULATOR_CONTROL_REGISTER    0x07
#define RECEIVER_CONTROL_REGISTER_1                 0x08
#define RECEIVER_CONTROL_REGISTER_2                 0x09
#define POWER_DOWN_CONTROL_REGISTER                 0x0A
#define STATE_REGISTER                              0x0F

#define RX5808_READ_CTRL_BIT                        0x00
#define RX5808_WRITE_CTRL_BIT                       0x01
#define RX5808_ADDRESS_R_W_LENGTH                   5
#define RX5808_DATA_LENGTH                          20
#define RX5808_PACKET_LENGTH                        25


static uint32_t rtc6715readRegister(uint8_t readRegister)
{
    uint32_t buf = readRegister | (RX5808_READ_CTRL_BIT << 4);
    uint32_t registerData = 0;

    uint32_t periodMicroSec = 100;

    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    // Write register address and read bit
    for (uint8_t i = 0; i < RX5808_ADDRESS_R_W_LENGTH; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        buf >>= 1; 
    }

    // Change pin from output to input
    pinMode(PIN_MOSI, INPUT);

    // Read data 20 bits
    for (uint8_t i = 0; i < RX5808_DATA_LENGTH; i++)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);

        if (digitalRead(PIN_MOSI))
        {
            registerData = registerData | (1 << (5 + i));
        }

        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);
    }

    // Change pin back to output
    pinMode(PIN_MOSI, OUTPUT);

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    return registerData;
}

static void rtc6715WriteRegister(uint32_t buf)
{
    uint32_t periodMicroSec = 100;

    digitalWrite(PIN_CS, LOW);
    #if defined(PIN_CS_2)
        digitalWrite(PIN_CS_2, LOW);
    #endif
    delayMicroseconds(periodMicroSec);

    for (uint8_t i = 0; i < RX5808_PACKET_LENGTH; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        buf >>= 1; 
    }

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);
    #if defined(PIN_CS_2)
        digitalWrite(PIN_CS_2, HIGH);
    #endif
}

void rtc6715SetFreq(uint16_t freq)
{
    uint32_t data = ((((freq - 479) / 2) / 32) << 7) | (((freq - 479) / 2) % 32);
    uint32_t newRegisterData = SYNTHESIZER_REG_B  | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);
    uint32_t currentRegisterData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | rtc6715readRegister(SYNTHESIZER_REG_B);
    if (newRegisterData != currentRegisterData)
    {
        rtc6715WriteRegister(SYNTHESIZER_REG_A  | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
        rtc6715WriteRegister(newRegisterData);
    }
}
