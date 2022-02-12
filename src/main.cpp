#define F_CPU (8000000LL)
#include <Arduino.h>
#include <TinyWireS.h>

constexpr uint8_t PIN_SDA = 0;
constexpr uint8_t PIN_SCL = 2;
constexpr uint8_t PIN_DRIVE = 1;
constexpr uint8_t PIN_NOT_EN = 4;
constexpr uint8_t PIN_LED = 3;

constexpr uint8_t I2C_ADDRESS = 0x42;

volatile uint8_t readn = 0;
volatile uint8_t speed = 127;

void init_pwm()
{
    TCCR1=(1<<PWM1A) | // PWM mode A
    (1<<COM1A1) |  //  OC1A out
    (1<<CS12) | (1<<CS11) | (1<<CS10); // clk/64
    //GTCCR=(1<<PWM1B) |  // PWM mode B
    //(1<<COM1B1); // OC1B out
}

void set_pwm(uint8_t out0, uint8_t out1)
{
    OCR1A=out0;
    OCR1B=255-out1;
}

void set_fan_speed(uint8_t speed)
{
    //uint8_t voltage_drive = 128 + ((speed > 127) ? (speed-128) : 0);
    uint8_t voltage_drive = 128 + speed/2;
    uint8_t output_drive = 127 + min(speed, 128);
    bool enabled = speed > 0;
    if(!enabled)
        output_drive = 0;
    set_pwm(voltage_drive, output_drive);
    digitalWrite(PIN_NOT_EN, !enabled);
}

void on_i2c_write(uint8_t n)
{
    while(TinyWireS.available())
    {
        speed = TinyWireS.receive();
    }
}
void on_i2c_read()
{
    readn = (readn+1) % 3;
    if(readn == 0)
        TinyWireS.send(0x42);
    else if(readn == 1)
        TinyWireS.send(0xFE);
    else if(readn == 2)
        TinyWireS.send(speed);
}

void setup() 
{
    static bool b = false;

    TinyWireS.begin(I2C_ADDRESS);
    TinyWireS.onReceive(on_i2c_write);
    TinyWireS.onRequest(on_i2c_read);
    pinMode(PIN_DRIVE, OUTPUT);
    pinMode(PIN_NOT_EN, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, 1);
    init_pwm();
    set_fan_speed(0);
    while(true)
    {
        set_fan_speed(speed);
        digitalWrite(PIN_LED, b=!b);
        delay(100);
        TinyWireS_stop_check();
    }
}
void loop() 
{
}