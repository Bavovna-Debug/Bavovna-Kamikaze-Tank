#undef T0

#include "Transmitter.h"

#ifdef T0
const int ThrottleStepUp    = 1;
const int ThrottleStepDown  = 5;
#else
const int ThrottleStepUp    = 12;
const int ThrottleStepDown  = 60;
#endif

const int TurningMinThrottle = 40;
const int TurningSkipLevel = 40;

extern volatile struct Signal signals[NumberOfSignals];

volatile int pwmLeft    = 0;
volatile int pwmRight   = 0;

static bool operational = false;
static int lastThrottle = 0;

void setup()
{
    DDRD |= (1 << PORTD2);
    DDRD |= (1 << PORTD3);

    setupTransmitter();

    cli();

#ifdef T0
    TCCR0A = (1 << WGM01);
    OCR0A = 0xF9;
    TIMSK0 |= (1 << OCIE0A);    
    TCCR0B |= (1 << CS01);
    TCCR0B |= (1 << CS00);
#endif

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 39999;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);  
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

void loop()
{
    bool allSignalsOK = true;
    for (byte signalNumber = 0; signalNumber < NumberOfSignals; signalNumber++)
    {
        if (signalNumber == 1)
        {
            continue;
        }

        if (signals[signalNumber].numberOfSamples < 10)
        {
            allSignalsOK = false;
        }

        signals[signalNumber].numberOfSamples = 0;
    }

    operational = allSignalsOK;

    delay(250);
}

#ifdef T0
static unsigned int ticker = 0;
#endif

#ifdef T0
ISR(TIMER0_COMPA_vect)
#else
void x()
#endif
{
#ifdef T0
    ticker++;
#endif

    signed int steeringLevel = signals[0].strobeDuration - 500;
    signed int throttleLevel = signals[2].strobeDuration;
    signed int turningLevel = signals[3].strobeDuration - 500;
    signed int throttleLimiter = signals[4].strobeDuration;
    signed int steeringCalibration = signals[5].strobeDuration - 500;

    signed int left = 0;
    signed int right = 0;

    steeringLevel += steeringCalibration / 10;

    if ((throttleLevel < TurningMinThrottle) &&
       ((turningLevel < -(TurningSkipLevel)) || (turningLevel > +(TurningSkipLevel))))
    {
        if (turningLevel < -(TurningSkipLevel))
        {
            left = 0;
            right = -(turningLevel) - TurningSkipLevel;
        }
        else if (turningLevel > +(TurningSkipLevel))
        {
            left = +(turningLevel) - TurningSkipLevel;
            right = 0;
        }
        else
        {
            left = 0;
            right = 0;
        }
    }
    else
    {
        if (throttleLevel > lastThrottle)
        {
    #ifdef T0
            if ((ticker % 2) == 0)
            {
                throttleLevel = lastThrottle + ThrottleStepUp;
            }
            else
            {
                throttleLevel = lastThrottle;
            }
    #else
            throttleLevel = lastThrottle + ThrottleStepUp;
    #endif
        }
        else if ((throttleLevel + ThrottleStepDown) < lastThrottle)
        {
            throttleLevel = lastThrottle - ThrottleStepDown;
        }

        lastThrottle = throttleLevel;

        left = throttleLevel;
        right = throttleLevel;

        if (steeringLevel > 0)
        {
            left -= steeringLevel;
        }
        else if (steeringLevel < 0)
        {
            right += steeringLevel;
        }
    }

    if (left < 0) left = 0;
    if (right < 0) right = 0;
    if (left > 1000) left = 1000;
    if (right > 1000) right = 1000;

    pwmLeft = map(left, 0, 1000, 1000, 1000 + throttleLimiter);
    pwmRight = map(right, 0, 1000, 1000, 1000 + throttleLimiter);
}

ISR(TIMER1_COMPA_vect)
{
#ifndef T0
    x();
#endif

    if (operational == false)
    {
        PORTD |= (1 << PORTB2);
        PORTD |= (1 << PORTB3);
        delayMicroseconds(1000);
        PORTD &= ~(1 << PORTD2);
        PORTD &= ~(1 << PORTD3);
    }
    else
    {
        if (pwmLeft < pwmRight)
        {
            PORTD |= (1 << PORTB2);
            PORTD |= (1 << PORTB3);
            delayMicroseconds(pwmLeft);
            PORTD &= ~(1 << PORTD2);
            delayMicroseconds(pwmRight - pwmLeft);
            PORTD &= ~(1 << PORTD3);
        }
        else if (pwmLeft > pwmRight)
        {
            PORTD |= (1 << PORTB3);
            PORTD |= (1 << PORTB2);
            delayMicroseconds(pwmRight);
            PORTD &= ~(1 << PORTD3);
            delayMicroseconds(pwmLeft - pwmRight);
            PORTD &= ~(1 << PORTD2);
        }
        else
        {
            PORTD |= (1 << PORTB2);
            PORTD |= (1 << PORTB3);
            delayMicroseconds(pwmLeft);
            PORTD &= ~(1 << PORTD2);
            PORTD &= ~(1 << PORTD3);
        }
    } 
}
