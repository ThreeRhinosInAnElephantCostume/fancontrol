#define F_CPU (8000000LL)
#include <Arduino.h>

constexpr uint8_t PIN_SDA = 0;
constexpr uint8_t PIN_SCL = 2;
constexpr uint8_t PIN_DRIVE = 1;
constexpr uint8_t PIN_NOT_EN = 4;
constexpr uint8_t PIN_LED = 3;

constexpr uint8_t REGISTERS_N = 2;

struct Config
{
    union
    {
        uint8_t data;
        struct
        {
            bool fan_enabled : 1;
            bool test : 1;
        };
    };
};
static_assert(sizeof(Config) == 1);

struct 
{
    union
    {
        uint8_t _regdata[REGISTERS_N];
        struct
        {
            uint8_t speed = 127;
            Config config;
        };
    };
}registers;

static_assert(sizeof(registers) == REGISTERS_N);

void init_pwm()
{
    TCCR1=(1<<PWM1A) | // PWM mode A
    (1<<COM1A1) |  //  OC1A out
    (1<<CS11) | (1<<CS10); // clk/4
    GTCCR=(1<<PWM1B) |  // PWM mode B
    (1<<COM1B1); // OC1B out
}

void set_pwm(uint8_t out0, uint8_t out1)
{
    OCR1A=out0;
    OCR1B=255-out1;
}

void set_fan_speed(uint8_t speed)
{
    uint8_t voltage_drive = 128 + ((speed > 127) ? (speed-127) : 0);
    uint8_t output_drive = 127 + min(speed, 128);
    bool enabled = speed == 0 || (uint8_t)(registers.config.fan_enabled);
    if(!enabled)
    {
        output_drive = 0;
        digitalWrite(PIN_NOT_EN, !enabled);
    }
    set_pwm(voltage_drive, output_drive);
}

void setup() 
{
    static bool b = false;
    registers.config.fan_enabled = true;
    registers.config.test =  true;
    pinMode(PIN_DRIVE, OUTPUT);
    pinMode(PIN_NOT_EN, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, 1);
    init_pwm();
    while(true)
    {
        if((registers.config.test))
        {
            for(uint8_t i = 0; i < 255; i++)
            {
                if(!registers.config.test)
                    break;
                set_fan_speed(i);
                digitalWrite(PIN_LED, !!(i % 2));
                delay(100);
            }
            set_fan_speed(0);
            digitalWrite(PIN_LED, 1);
            delay(1000);
        }
        else
        {
            set_fan_speed(registers.speed);
            digitalWrite(PIN_LED, b=!b);
            delay(100);
        }
    }
}
void loop() 
{
}