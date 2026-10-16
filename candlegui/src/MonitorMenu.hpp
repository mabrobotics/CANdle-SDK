#include <iostream>

#include "imgui.h"
#include "implot.h"

#include "commonMemory.hpp"

class MonitorMenu
{
  public:
    MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawMonitorMenu();

    // Draw plots

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    std::vector<MDStatus_S> MonitorMenuMDStatus;

    // Dimensions of windows
    float canBusInfoBarHeight   = 170.0f;
    float actuatorsMenuBarWidth = 325.0f;
    float plotsMenuWidth        = 1200.0f;
    float liveValuesMenuHeight  = 170.0f;

    // Plots settings

    // Colors
    const ImVec4 redColor   = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 greenColor = ImVec4(0.30f, 0.80f, 0.40f, 1.0f);

    // Monitor Menu Draw Functions
    void drawActuatorMenu();
    void drawCanBusInfoMenu();
    void drawPlotsMenu();
    void drawLiveValuesMenu();
    void drawPlaceHolderMenu();

    // Draw Plots
    void drawDevicePlot(PlotSignal_E signal, const char* plotId, const char* yLabel);

    void drawVelocityPlots()
    {
        drawDevicePlot(PlotSignal_E::VELOCITY, "##VelocityPlot", "Velocity [rad/s]");
    }
    void drawPositionPlots()
    {
        drawDevicePlot(PlotSignal_E::POSITION, "##PositionPlot", "Position [rad]");
    }
    void drawTorquePlots()
    {
        drawDevicePlot(PlotSignal_E::TORQUE, "##TorquePlot", "Torque [Nm]");
    }
    void drawMosfetTempPlots()
    {
        drawDevicePlot(PlotSignal_E::MOSFET_TEMP, "##MosfetTempPlot", "T MOSFET [C]");
    }

    // Help functions
    void refreshMDStatus();
    void cellFloat(float v, const char* fmt);
};