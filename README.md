# Spaghetti copypasta code that makes a Mac understand i-bus data coming from a Flysky receiver through serial as joystick input.

1. Connect receiver through a serial device (FTDI, Arduino,...) to Mac
2. Edit serial device name in `ibus_vjoy.py`
3. Compile: `g++ main.cpp -o vjoy -framework IOKit -std=c++11`
4. `python ibus_vjoy.py`
5. Enjoy FreeRider !
