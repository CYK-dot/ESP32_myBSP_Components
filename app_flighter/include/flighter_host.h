#pragma once

#include "flighter_ptl.h"

#ifdef __cplusplus
    extern "C" {
#endif


#ifdef __cplusplus
    }
#endif

typedef struct {
    float rockLX;
    float rockLY;
    float rockRX;
    float rockRY;
    uint8_t keyJ1 : 1;
    uint8_t keyJ2 : 1;
    uint8_t keyL1 : 1;
    uint8_t keyL2 : 1;
    uint8_t keyL3 : 1;
    uint8_t keyL4 : 1;
    uint8_t keyR1 : 1;
    uint8_t keyR2 : 1;
    uint8_t keyR3 : 1;
    uint8_t keyR4 : 1;
}FlighterKeyboard_t;

void FlighterHostSetup(void);
void FlighterHostKeyboardUpdate(uint16_t tickToWait,FlighterKeyboard_t* output);
