const char FOOHID_SERVICE_NAME[] = "it_unbit_foohid";

enum class FooHIDMethod {
    Create = 0,
    Destroy,
    Send,
};

static unsigned char report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
 
    0x05, 0x01,                    //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE
    0x09, 0x33,                    //     USAGE
    0x15, 0x81,                    //   LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x04,                    //     REPORT_COUNT (4)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
   
    0x05, 0x09,                    //   USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //   USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x55, 0x00,                    //   UNIT_EXPONENT (0)
    0x65, 0x00,                    //   UNIT (None)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

static bool connectToService(io_connect_t *connection, std::string *errorMessage)
{
    io_iterator_t iterator;
    io_service_t service;
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     IOServiceMatching(FOOHID_SERVICE_NAME),
                                                     &iterator);
    if (ret != KERN_SUCCESS) {
        *errorMessage = "Unable to find FooHID IOService.";
        return false;
    }
    bool found = false;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, connection);
        if (ret == KERN_SUCCESS) {
            found = true;
            break;
        }
        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);
    if (!found) {
        *errorMessage = "Unable to connect to FooHID IOService.";
        return false;
    }
    return true;
}

static void disconnectFromService(io_connect_t connection)
{
    IOServiceClose(connection);
}

FooHIDJoystick::FooHIDJoystick(const std::string &name, const std::string &serialNumber)
    : name(name), serialNumber(serialNumber)
{
    connectionOpened = connectToService(&connection, &_errorMessage);
    _hasError = !connectionOpened;
    if (!_hasError) {
        destroyDevice();
        deviceCreated = createDevice();
        _hasError = !deviceCreated;
        if (!deviceCreated)
            _errorMessage = "Failed to create virtual joystick";
    }
}

FooHIDJoystick::~FooHIDJoystick()
{
    if (deviceCreated)
        destroyDevice();
    if (connectionOpened)
        disconnectFromService(connection);
}

bool FooHIDJoystick::hasError() const
{
    return _hasError;
}

std::string FooHIDJoystick::errorMessage() const
{
    return _errorMessage;
}

void FooHIDJoystick::setValue(JoystickValues newValues)
{
    values = newValues;
    if (!sendToDevice()) {
        _hasError = true;
        _errorMessage = "Failed to send values to virtual joystick";
    }
}

bool FooHIDJoystick::createDevice() const
{
    uint64_t params[8];
    params[0] = uint64_t(name.c_str());
    params[1] = uint64_t(name.size());
    params[2] = uint64_t(report_descriptor);
    params[3] = sizeof(report_descriptor);
    params[4] = uint64_t(serialNumber.c_str());
    params[5] = uint64_t(serialNumber.size());
    params[6] = uint64_t(2);
    params[7] = uint64_t(3);

    kern_return_t ret = IOConnectCallScalarMethod(connection, int(FooHIDMethod::Create), params,
                                                  sizeof(params)/sizeof(params[0]), NULL, 0);
    return ret == KERN_SUCCESS;
}

bool FooHIDJoystick::sendToDevice() const
{
    uint64_t params[4];
    params[0] = uint64_t(name.c_str());
    params[1] = uint64_t(name.size());
    params[2] = uint64_t(&values);
    params[3] = sizeof(struct JoystickValues);
    kern_return_t ret = IOConnectCallScalarMethod(connection, int(FooHIDMethod::Send), params,
                                                  sizeof(params)/sizeof(params[0]), NULL, 0);
    return ret == KERN_SUCCESS;
}

void FooHIDJoystick::destroyDevice() const
{
    uint64_t params[2];
    params[0] = uint64_t(name.c_str());
    params[1] = uint64_t(name.size());
    IOConnectCallScalarMethod(connection, int(FooHIDMethod::Destroy), params,
                              sizeof(params)/sizeof(params[0]), NULL, 0);
}
