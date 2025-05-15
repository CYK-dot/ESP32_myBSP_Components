#pragma once

typedef struct {
    float rockLX;
    float rockLY;
    float rockRX;
    float rockRY;

    bool keyL1 : 1;
    bool keyL2 : 1;
    bool keyL3 : 1;
    bool keyL4 : 1;

    bool keyR1 : 1;
    bool keyR2 : 1;
    bool keyR3 : 1;
    bool keyR4 : 1;

    bool keyC1 : 1;
    bool keyC2 : 1;
    bool keyJ1 : 1;
    bool keyJ2 : 1;

    bool CtrlTimeout : 1;
}FlighterPackage_t;