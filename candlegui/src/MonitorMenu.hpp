#include <iostream>

#include "BusActuatorsMenu.hpp"
#include "imgui.h"
#include "implot.h"

#include "commonMemory.hpp"
#include <filesystem>

class MonitorMenu
{
  public:
    MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawMonitorMenu();
    void refreshRecordingList();
    void updateRecordingState();
    // Record timer
    std::chrono::steady_clock::time_point recordTimeStart;

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    std::vector<MDStatus_S> MonitorMenuMDStatus;

    // Dimensions of windows
    float canBusInfoBarHeight  = 170.0f;
    float liveValuesMenuHeight = 170.0f;

    // Recording variables
    std::vector<std::string> recordingFiles;
    int                      selectedRecording = -1;
    bool                     guiWasRecording   = false;
    bool                     exportOk          = false;

    // Testing
    std::filesystem::path exportDir;
    std::filesystem::path browsePath;
    char                  pathBuf[512] = {};
    bool                  pathError    = false;

    // Monitor Menu Draw Functions
    void drawActuatorMenu();
    void drawCanBusInfoMenu();
    void drawPlotsMenu();
    void drawLiveValuesMenu();
    void drawRecordDataMenu();

    void drawRecordingButtons();
    void drawExportButtons();

    bool drawFolderBrowserPopup(const char* popupId, std::filesystem::path& result);

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
    void                  refreshMDStatus();
    void                  cellFloat(float v, const char* fmt);
    void                  setBrowsePath(const std::filesystem::path& path);
    std::filesystem::path defaultExportDir();
};