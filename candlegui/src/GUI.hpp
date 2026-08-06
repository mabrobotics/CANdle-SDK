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

    float leftMenuBarWidth    = 350.0f;
    float rightMenuBarWidth   = 300.0f;
    float testMenuBarHeight   = 100.0f;
    float errorMenuBarHeight  = 50.0f;
    float lowBarHeight        = 30.0f;
    float marginPlot          = 10.f;
    float paddingButtons      = 30.f;
    float mediumButtonHeight  = 40.0f;
    float roundingFrameButton = 12.0f;

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

    std::string chosenIDstr = "Select Your MD";

    const float targetHoldTime = 0.25f;

    const float step      = 0.1f;
    const float step_fast = 1.0f;

    // Main menu draw functions
    void drawMenuTopBar();
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
    void drawVelocityPlot();
    void drawPositionPlot();
    void drawTorquePlot();

    // Style edit
    void        comboStyle(const char* text);
    void        buttonStyle();
    void        endComboStyle();
    void        endButtonStyle();
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