#include <iostream>
#include <string>
#include "vjoy.h"
#include "vjoy.cpp"


int main(int argc, char **argv) {

    FooHIDJoystick* joystick = new FooHIDJoystick("Virtual TX", "SN 123456");
    
    if (joystick->hasError())
        printf("%s\n", joystick->errorMessage().c_str());

    char buf[2];
    int cnt = 0;
    JoystickValues* newValues = new JoystickValues;
    while (true) {
        std::cin.read(buf, 2);
        unsigned char h = buf[0];
        unsigned char l = buf[1];
        uint16_t c = h << 8 | l;
        int8_t v = (((c - 1000) * 254) / 1000) - 127;
        if (cnt == 0) newValues->x = v;
        if (cnt == 1) newValues->y = v;
        if (cnt == 2) newValues->z = v;
        if (cnt == 3) newValues->rx = v;
        cnt++;
        if (cnt == 8) {
            joystick->setValue(*newValues);
            // printf("%d,%d,%d,%d\n", newValues->x, newValues->y, newValues->z, newValues->rx);
            cnt = 0;
        }
    }
}
