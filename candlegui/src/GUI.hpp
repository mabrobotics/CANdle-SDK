#include <GLFW/glfw3.h>

// IMgui library
#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <filesystem>
#include "commonMemory.hpp"

class GraphicInterface
{
  public:
    GraphicInterface(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void init();
    void close();
    void loop();

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    GLFWwindow* m_window = nullptr;

    float leftMenuBarWidth      = 325.0f;
    float rightMenuBarWidth     = 175.0f;
    float testMenuBarHeight     = 100.0f;
    float errorMenuBarHeight    = 50.0f;
    float lowBarHeight          = 30.0f;
    float marginPlot            = 10.f;
    float paddingButtons        = 30.f;
    float rightMenuButtonWidth  = 150.f;
    float mediumButtonHeight    = 40.0f;
    float roundingFrameButton   = 12.0f;
    float roundingFrameCheckbox = 8.0f;

    float resizeButton = 15.0f;

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

    // Back menu settings
    ImGuiWindowFlags flagsBackMenu = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Table settings
    ImGuiTableFlags flagsTables = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImVec4 clear_color2 = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 clear_color  = ImVec4(0.055f, 0.059f, 0.067f, 1.00f);
    ImVec4 buttonColor  = ImVec4(0.167f, 0.165f, 0.196f, 1.0f);

    ImVec4 mabColor        = ImVec4(1.0f, 0.468f, 0.0f, 1.0f);
    ImVec4 mabColorHovered = ImVec4(1.0f, 0.468f, 0.0f, 0.5f);

    const ImVec4 colA = mabColor;
    const ImVec4 colB = mabColor;

    std::string chosenIDstr = "Select Your MD";

    const float targetHoldTime = 0.25f;

    const float step      = 0.1f;
    const float step_fast = 1.0f;

    // Main menu draw functions
    void drawErrorMenuBar();
    void drawMenuLowerBar();
    void drawTestMenuBar();
    void drawLeftMenuBar();
    void drawRightMenuBar();
    void drawMainMenu();
    void drawErrorMenuPopup();

    // Buttons
    void drawTestManualButton();
    void drawTestEndButton();
    void drawDiscoverMDButton();
    void drawSelectMDButton();
    void drawSelectModeButton();
    void drawRestorePlotsButton();
    void drawCheckboxCrosshairsButton();
    void drawCheckboxVerCursorsVelButton();
    void drawCheckboxVerCursorsPosButton();
    void drawCheckboxVerCursorsTrqButton();
    void drawCheckboxHorCursorsVelButton();
    void drawCheckboxHorCursorsPosButton();
    void drawCheckboxHorCursorsTrqButton();

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
    void timeInTarget(bool& inWindow, float& timeInTargetWindow, float& dt);
    void drawVelocityPlot(ImVec2 size, ImPlotFlags plotFlag);
    void drawPositionPlot(ImVec2 size, ImPlotFlags plotFlag);
    void drawTorquePlot(ImVec2 size, ImPlotFlags plotFlag);

    // Draw values
    void drawValuesVelocity();
    void drawValuesPosition();
    void drawValuesTorque();

    // Style edit
    void        comboStyle(const char* text);
    void        buttonStyle();
    void        checkboxStyle();
    void        endComboStyle();
    void        endButtonStyle();
    void        endCheckboxStyle();
    void        centerText(const char* text);
    const char* getModeName(mab::MdMode_E mode);
    bool        drawBigInputFloat(const char* label,
                                  float*      v,
                                  float       step,
                                  float       step_fast,
                                  const char* format,
                                  float       windowWidth,
                                  const char* unit = nullptr);
    bool        buttonColorInputFloat(
        const char* label, float* v, float step, float step_fast, const char* format);
};