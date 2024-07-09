#include <stdint.h>

class iLoader
{
  public:
    enum class Error_E : uint8_t
    {
        OK = 0,
        ERROR_UNKNOWN,
    };

    virtual Error_E enterBootloader() = 0;
    virtual Error_E uploadFirmware()  = 0;
    virtual Error_E sendBootCommand() = 0;
};