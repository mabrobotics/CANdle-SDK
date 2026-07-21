// IMgui library
#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// MAB Sdk
#include "candle.hpp"
#include "MD.hpp"

#include <stdio.h>
#include <math.h>

#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

struct plotPoints_S
{
    float time;

    float velocity;
    float position;
    float torque;

    float targetVelocity;
    float targetPosition;
    float targetTorque;
};

struct guiBuffers_S
{
    static const uint32_t SIZE = 100000;

    float guiElapsedTime = 0.0f;

    float time[SIZE]      = {0};
    float vel[SIZE]       = {0};
    float pos[SIZE]       = {0};
    float trq[SIZE]       = {0};
    float targetVel[SIZE] = {0};
    float targetPos[SIZE] = {0};
    float targetTrq[SIZE] = {0};

    uint32_t offset   = 0;
    uint32_t readData = 0;

    float minVel = 0.0f, maxVel = 0.0f;
    float minPos = 0.0f, maxPos = 0.0f;
    float minTrq = 0.0f, maxTrq = 0.0f;

    void reset()
    {
        offset = 0;
        maxVel = 0.0f;
        minVel = 0.0f;
        maxTrq = 0.0f;
        minTrq = 0.0f;
        maxPos = 0.0f;
        minPos = 0.0f;
        for (uint32_t i = 0; i < SIZE; ++i)
        {
            time[i] = vel[i] = pos[i] = trq[i] = targetVel[i] = 0.0f;
        }
    }
};

struct commonMemory_S
{
    std::atomic<int> actual_thread_hz{0};
    std::mutex       mtx;
    float            targetVelocity     = 0.0f;
    float            targetPosition     = 0.0f;
    float            targetTorque       = 0.0f;
    float            targetAcceleration = 0.0f;
    float            targetDeceleration = 0.0f;

    // TUNING GAINS
    float Kp_vel          = 0.0f;
    float Ki_vel          = 0.0f;
    float Kd_vel          = 0.0f;
    float integralMax_vel = 0.0f;

    float Kp_pos          = 0.0f;
    float Ki_pos          = 0.0f;
    float Kd_pos          = 0.0f;
    float integralMax_pos = 0.0f;

    float Kp_imp = 0.0f;
    float Kd_imp = 0.0f;

    // MAB
    mab::MdMode_E             currentMode = mab::MdMode_E::IDLE;
    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID = 0;

    // Logic
    std::atomic<bool> testStarted{false};
    std::atomic<bool> buttonDiscoverMdPressed{false};
    std::atomic<bool> updateParametersTest{false};
    std::atomic<bool> selectedMDid{false};
    std::atomic<bool> testOngoing{false};

    // Plots
    static const uint32_t PLOT_BUFFER_SIZE = 100000;
    plotPoints_S          plotBuffer[PLOT_BUFFER_SIZE];
    std::atomic<uint32_t> plotWriteData{0};
};

static bool systemON        = true;
static bool selectedMD      = false;
static bool selectedMode    = false;
static bool discoverOngoing = false;

static int   monitorX, monitorY;
static float leftMenuBarWidth    = 500.0f;
static float testMenuBarHeight   = 100.0f;
static float lowBarHeight        = 30.0f;
static float marginPlot          = 10.f;
static float paddingButtons      = 30.f;
static float mediumButtonHeight  = 40.0f;
static float roundingFrameButton = 12.0f;

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

// MAXIMUM VALUES
static float maxVelocityClamp     = 0.0f;
static float maxPositionClamp     = 0.0f;
static float minPositionClamp     = 0.0f;
static float maxTorqueClamp       = 0.0f;
static float maxAccelerationClamp = 0.0f;
static float maxDecelerationClamp = 0.0f;

const float targetHoldTime = 0.25f;

static float positionWindowSlider = 0.0f;
static float positionWindow       = 0.1f;
static float velocityWindowSlider = 0.0f;
static float velocityWindow       = 0.5f;

static float Kp_velSlider          = 0.0f;
static float Ki_velSlider          = 0.0f;
static float Kd_velSlider          = 0.0f;
static float integralMax_velSlider = 0.0f;

static float Kp_posSlider          = 0.0f;
static float Ki_posSlider          = 0.0f;
static float Kd_posSlider          = 0.0f;
static float integralMax_posSlider = 0.0f;

static float Kp_impSlider = 0.0f;
static float Kd_impSlider = 0.0f;

static float targetVelocitySlider     = 0.0f;
static float targetPositionSlider     = 0.0f;
static float targetTorqueSlider       = 0.0f;
static float targetAccelerationSlider = 0.0f;
static float targetDecelerationSlider = 0.0f;
const float  step                     = 0.1f;
const float  step_fast                = 1.0f;

// functions
static void updatePlotData(commonMemory_S& memory, guiBuffers_S& buffers, ImGuiIO& io);
static void drawMenuTopBar(commonMemory_S& memory, ImGuiIO& io);
static void drawMenuLowerBar(commonMemory_S& memory, ImGuiIO& io);
static void drawMainMenu(commonMemory_S& memory, ImGuiIO& io, guiBuffers_S& buffers);
static void drawLeftMenuBar(commonMemory_S& memory, ImGuiIO& io);
static void drawTestMenuBar(commonMemory_S& memory, ImGuiIO& io);

static void timeInTarget(bool&           inWindow,
                         float&          timeInTargetWindow,
                         float&          dt,
                         commonMemory_S& memory);
static void drawVelocityPlot(commonMemory_S& memory, guiBuffers_S& buffers);
static void drawPositionPlot(commonMemory_S& memory, guiBuffers_S& buffers);
static void drawTorquePlot(commonMemory_S& memory, guiBuffers_S& buffers);

static void drawSetTargetVelocity();
static void drawSetTargetPosition();
static void drawSetTargetTorque();
static void drawSetTargetAcceleration();
static void drawSetTargetDeceleration();
static void drawSetPositionWindow();
static void drawSetVelocityWindow();

static void drawTestEndButton(commonMemory_S& memory);
static void drawDiscoverMDButton(commonMemory_S& memory);

static void drawParametersVelocity(commonMemory_S& memory);
static void drawParametersPosition(commonMemory_S& memory);
static void drawParametersImpedance(commonMemory_S& memory);

static void drawSelectModeButton(commonMemory_S& memory);
static void drawSelectMDButton(commonMemory_S& memory);

static void comboStyle(const char* text);
static void buttonStyle();
static void endComboStyle();
static void endButtonStyle();
static void centerText(const char* text);
const char* getModeName(mab::MdMode_E mode);
static bool drawBigInputFloat(const char* label,
                              float*      v,
                              float       step,
                              float       step_fast,
                              const char* format,
                              float       windowWidth);
static bool buttonColorInputFloat(
    const char* label, float* v, float step, float step_fast, const char* format);

static void testMD(commonMemory_S& memory, mab::MD& md);
static void downloadParameters(commonMemory_S& memory, mab::MD& md);

static void updateVelParameters(commonMemory_S& memory);
static void updatePosParameters(commonMemory_S& memory);
static void updateImpParameters(commonMemory_S& memory);

// hardware thread loop
void candleLoop(commonMemory_S& memory, std::atomic<bool>& isRunning);