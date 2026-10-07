#include <iostream>

#include "imgui.h"
#include "implot.h"

#include "commonMemory.hpp"

class MonitorMenu
{
  public:
    MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    // Monitor Menu Draw Functions
    void drawMonitorMenu();
    void drawActuatorMenu();
    void drawCanBusInfoMenu();

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    // Dimensions of windows
    float canBusInfoBarHeight   = 170.0f;
    float actuatorsMenuBarWidth = 325.0f;
};