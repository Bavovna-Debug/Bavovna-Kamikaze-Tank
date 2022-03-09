#include "Transmitter.h"

volatile struct Signal signals[NumberOfSignals];

void setupTransmitter(void)
{
    memset((void*) &signals, 0, sizeof(signals));

    DDRB &= ~(1 << PORTB0);
    DDRB &= ~(1 << PORTB1);
    DDRB &= ~(1 << PORTB2);
    DDRB &= ~(1 << PORTB3);
    DDRB &= ~(1 << PORTB4);
    DDRB &= ~(1 << PORTB5);

    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT0);
    PCMSK0 |= (1 << PCINT1);
    PCMSK0 |= (1 << PCINT2);
    PCMSK0 |= (1 << PCINT3);
    PCMSK0 |= (1 << PCINT4);
    PCMSK0 |= (1 << PCINT5);
}

ISR(PCINT0_vect)
{
    int mask = PINB;
    unsigned long timestamp = micros();

    if ((mask & (1 << PINB0)) == (1 << PINB0))
    {
        strobeUp(0, timestamp);
    }
    else
    {
        strobeDown(0, timestamp);
    }

    if ((mask & (1 << PINB1)) == (1 << PINB1))
    {
        strobeUp(1, timestamp);
    }
    else
    {
        strobeDown(1, timestamp);
    }

    if ((mask & (1 << PINB2)) == (1 << PINB2))
    {
        strobeUp(2, timestamp);
    }
    else
    {
        strobeDown(2, timestamp);
    }

    if ((mask & (1 << PINB3)) == (1 << PINB3))
    {
        strobeUp(3, timestamp);
    }
    else
    {
        strobeDown(3, timestamp);
    }

    if ((mask & (1 << PINB4)) == (1 << PINB4))
    {
        strobeUp(4, timestamp);
    }
    else
    {
        strobeDown(4, timestamp);
    }

    if ((mask & (1 << PINB5)) == (1 << PINB5))
    {
        strobeUp(5, timestamp);
    }
    else
    {
        strobeDown(5, timestamp);
    }
}

void strobeUp(const byte signalNumber, const unsigned long timestamp)
{
    if (signals[signalNumber].measurement == false)
    {
        signals[signalNumber].measurement = true;
        signals[signalNumber].strobeBegin = timestamp;
        signals[signalNumber].numberOfSamples++;
    }  
}

void strobeDown(const byte signalNumber, const unsigned long timestamp)
{
    if (signals[signalNumber].measurement == true)
    {
        signals[signalNumber].measurement = false;
        signals[signalNumber].strobeEnd = timestamp;

        signals[signalNumber].strobeDuration = signals[signalNumber].strobeEnd - signals[signalNumber].strobeBegin;

        if (signals[signalNumber].strobeDuration < 1000u)
        {
            signals[signalNumber].strobeDuration = 0u;
        }
        else if (signals[signalNumber].strobeDuration > 2000u)
        {
            signals[signalNumber].strobeDuration = 1000u;
        }
        else
        {
            signals[signalNumber].strobeDuration -= 1000u;
        }
    }
}
