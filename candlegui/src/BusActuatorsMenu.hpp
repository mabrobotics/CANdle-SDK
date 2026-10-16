#pragma once
#include <memory>

#include "commonMemory.hpp"
#include "GuiStyle.hpp"

class BusActuatorMenu : private GuiStyle
{
  public:
    BusActuatorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawBusActuatorMenu();

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    // Dimensions of windows
    float candleInfoMenuHeight = 240.0f;

    // Bus & Actuators draw functions
    void drawCANdleInfoMenu();
    void drawTestGroupsMenu();
    void drawFoundOnBusMenu();
};