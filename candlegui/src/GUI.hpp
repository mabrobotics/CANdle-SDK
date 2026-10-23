#include <GLFW/glfw3.h>

// IMgui library
#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <filesystem>
#include "commonMemory.hpp"
#include "MonitorMenu.hpp"
#include "BusActuatorsMenu.hpp"

class GraphicInterface
{
  public:
    GraphicInterface(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void init();
    void close();
    void loop();

  private:
    enum SelectedPage_E : uint8_t
    {
        TUNING      = 0,
        MONITOR     = 1,
        TEST_SCRIPT = 2,
        BUS_INFO    = 3
    };
    SelectedPage_E currentPage = TUNING;

    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    GLFWwindow* m_window = nullptr;

    MonitorMenu     m_monitorWindow;
    BusActuatorMenu m_busActuatorWindow;

    std::vector<MDStatus_S> guiMDStatus;

    float leftMenuBarHeight;
    float rightMenuBarWidth  = 250.0f;
    float testMenuBarHeight  = 0.0f;
    float errorMenuBarHeight = 40.0f;
    float topBarHeight       = 40.0f;
    float lowBarHeight       = 20.0f;
    float marginPlot         = 10.f;

    float rightMenuButtonWidth = rightMenuBarWidth - 20.f;

    float saveButtonWidth = GuiStyle::leftMenuBarWidth - 100.f;
    float resizeButton    = 15.0f;

    float menuTopHeightRatio   = 0.5f;
    float menuBottomWidthRatio = 0.5f;

    bool buttonRestorePlotsPressed   = false;
    bool buttonShowCrosshairsChecked = false;

    // Cursors

    float timeElapsedOnPlot;

    double minVel, maxVel;
    double minPos, maxPos;
    double minTrq, maxTrq;

    double cursorAPositionVel;
    double cursorBPositionVel;
    double cursorAPositionPos;
    double cursorBPositionPos;
    double cursorAPositionTrq;
    double cursorBPositionTrq;

    double cursorAHorPositionVel;
    double cursorBHorPositionVel;
    double cursorAHorPositionPos;
    double cursorBHorPositionPos;
    double cursorAHorPositionTrq;
    double cursorBHorPositionTrq;

    void resetCursors()
    {
        cursorAPositionVel = 0.0;
        cursorBPositionVel = 0.0;
        cursorAPositionPos = 0.0;
        cursorBPositionPos = 0.0;
        cursorAPositionTrq = 0.0;
        cursorBPositionTrq = 0.0;

        cursorAHorPositionVel = 0.0;
        cursorBHorPositionVel = 0.0;
        cursorAHorPositionPos = 0.0;
        cursorBHorPositionPos = 0.0;
        cursorAHorPositionTrq = 0.0;
        cursorBHorPositionTrq = 0.0;
    }

    bool cursorsVerticalVelocityEnabled = false;
    bool cursorsVerticalPositionEnabled = false;
    bool cursorsVerticalTorqueEnabled   = false;

    bool cursorsHorizontalVelocityEnabled = false;
    bool cursorsHorizontalPositionEnabled = false;
    bool cursorsHorizontalTorqueEnabled   = false;

    bool showCursorMenu = false;

    // Table settings
    ImGuiTableFlags flagsTables = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    std::string chosenIDstr = "Select Your MD";

    const float targetHoldTime = 0.25f;

    const float step      = 0.1f;
    const float step_fast = 1.0f;

    const char* getModeName(mab::MdMode_E mode);

    // Main menu draw functions
    void drawErrorMenuBar();
    void drawMenuTopBar();
    void drawMenuBottomBar();
    void drawTestMenuBar();
    void drawLeftMenuBar();
    void drawRightMenuBar();
    void drawMainMenu();
    void drawErrorPopup();

    void drawTopCANdleStatusMenu();

    void drawTuningMenu();
    void drawMonitorButton();
    void drawBusInfoButton();
    void drawTestScriptButton();

    // Buttons
    void drawTuningButton();

    void drawTestManualButton();
    void drawTestEndButton();
    void drawDiscoverMDButton();
    void drawSelectMDButton();
    void drawSelectModeButton();
    void drawRestorePlotsButton();
    void drawClearErrorsButton();
    void drawSaveButton();

    void drawCheckboxCrosshairsButton();
    void drawCheckboxVerCursorsVelButton();
    void drawCheckboxVerCursorsPosButton();
    void drawCheckboxVerCursorsTrqButton();
    void drawCheckboxHorCursorsVelButton();
    void drawCheckboxHorCursorsPosButton();
    void drawCheckboxHorCursorsTrqButton();
    void drawCursorMenu();

    // Helper
    void drawHelper(const char* description);

    // Parameters setters etc.
    void drawParametersVelocity();
    void drawParametersPosition();
    void drawParametersImpedance();

    void drawSetTargetVelocity();
    void drawSetTargetPosition();
    void drawSetTargetTorque();
    void drawSetTargetAcceleration();
    void drawSetTargetDeceleration();
    void drawSetPositionWindow();
    void drawSetVelocityWindow();

    // Draw plots
    void updatePlotData();
    void refreshMDStatus();
    void timeInTarget(bool& inWindow, float& timeInTargetWindow, float& dt);
    void drawVelocityPlot(ImVec2 size, ImPlotFlags plotFlag);
    void drawPositionPlot(ImVec2 size, ImPlotFlags plotFlag);
    void drawTorquePlot(ImVec2 size, ImPlotFlags plotFlag);
    void plotCursors(double& xA,
                     double& xB,
                     double& yA,
                     double& yB,
                     bool&   verticalCursors,
                     bool&   horizontalCursors);

    // Draw values
    void drawValuesVelocity();
    void drawValuesPosition();
    void drawValuesTorque();
};