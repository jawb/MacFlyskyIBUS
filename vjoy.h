#include <IOKit/IOKitLib.h>

struct JoystickValues {
    int8_t x;
    int8_t y;
    int8_t z;
    int8_t rx;
    int8_t buttons;
};

typedef unsigned char byte;

class FooHIDJoystick
{
public:
    FooHIDJoystick(const std::string &name, const std::string &serialNumber);
    ~FooHIDJoystick();

    bool hasError() const;
    std::string errorMessage() const;
    void setValue(JoystickValues newValues);

private:
    bool createDevice() const;
    bool sendToDevice() const;
    void destroyDevice() const;

    const std::string name;
    const std::string serialNumber;
    io_connect_t connection = 0;
    JoystickValues values = {127, 127};
    std::string _errorMessage;
    bool _hasError = true;
    bool connectionOpened = false;
    bool deviceCreated = false;
};
