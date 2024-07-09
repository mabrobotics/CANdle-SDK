#include "iLoader.hpp"

class CanLoader : public iLoader
{
  public:
    CanLoader();
    ~CanLoader();
    Error_E enterBootloader() override;
    Error_E uploadFirmware() override;
    Error_E sendBootCommand() override;
};