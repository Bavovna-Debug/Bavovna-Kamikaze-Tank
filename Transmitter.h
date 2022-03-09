#pragma once

#include <stdbool.h>

const byte NumberOfSignals = 6;

struct Signal
{
    bool measurement;
    unsigned int numberOfSamples;
    unsigned long strobeBegin;
    unsigned long strobeEnd;
    unsigned int strobeDuration;
};

void setupTransmitter(void);
