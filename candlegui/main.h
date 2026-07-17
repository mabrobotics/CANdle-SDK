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

struct CommonMemory
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

    // Hardware measurements
    float currentVelMeasured    = 0.0f;
    float currentPosMeasured    = 0.0f;
    float currentTorqueMeasured = 0.0f;

    // MAB
    std::mutex                mtx_mab;
    mab::MdMode_E             currentMode = mab::MdMode_E::IDLE;
    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID = 0;

    // Logic
    std::atomic<bool> testStarted{false};
    std::atomic<bool> buttonDiscoverMdPressed{false};
    std::atomic<bool> updateParametersTest{false};
    std::atomic<bool> selectedMDid{false};
    std::atomic<bool> testOngoing{false};
};

static bool systemON        = true;
static bool selectedMD      = false;
static bool selectedMode    = false;
static bool discoverOngoing = false;

static int   monitorX, monitorY;
static float leftMenuBarWidth  = 500.0f;
static float testMenuBarHeight = 100.0f;
static float margin            = 10.f;

// Back menu settings
ImGuiWindowFlags flagsBackMenu = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

// Table settings
ImGuiTableFlags flagsTables = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

ImColor mabColor = ImColor::HSV(0.078f, 1.0f, 1.0f);

std::string chosenIDstr = "Select Your MD";

// MAXIMUM VALUES
static float maxVelocityClamp     = 0.0f;
static float maxPositionClamp     = 0.0f;
static float minPositionClamp     = 0.0f;
static float maxTorqueClamp       = 0.0f;
static float maxAccelerationClamp = 0.0f;
static float maxDecelerationClamp = 0.0f;

const float refreshRate = 200.f;

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
static void drawMenuTopBar(CommonMemory& memory, ImGuiIO& io);
static void drawMainMenu(CommonMemory& memory, ImGuiIO& io);
static void drawLeftMenuBar(CommonMemory& memory, ImGuiIO& io);
static void drawTestMenuBar(CommonMemory& memory, ImGuiIO& io);

static void drawVelocityPlot(CommonMemory& memory, ImGuiIO& io);
static void drawPositionPlot(CommonMemory& memory, ImGuiIO& io);
static void drawTorquePlot(CommonMemory& memory, ImGuiIO& io);

static void drawSetTargetVelocity();
static void drawSetTargetPosition();
static void drawSetTargetTorque();
static void drawSetTargetAcceleration();
static void drawSetTargetDeceleration();
static void drawSetPositionWindow();
static void drawSetVelocityWindow();

static void drawTestButton(CommonMemory& memory);
static void drawEndTestButton(CommonMemory& memory);
static void drawDiscoverMDButton(CommonMemory& memory);

static void drawParametersVelocity(CommonMemory& memory);
static void drawParametersPosition(CommonMemory& memory);
static void drawParametersImpedance(CommonMemory& memory);

static void drawToggleButton();
static void drawSelectModeButton(CommonMemory& memory);
static void drawSelectMDButton(CommonMemory& memory);

static void CenterText(const char* text);
const char* getModeName(mab::MdMode_E mode);
static bool drawBigInputFloat(
    const char* label, float* v, float step, float step_fast, const char* format);
// static bool drawOrangeInputFloat(const char* label,
//                                  float*      v,
//                                  float       step      = 0.0f,
//                                  float       step_fast = 0.0f,
//                                  const char* format    = "%.3f");

static void testMD(CommonMemory& memory, mab::MD& md);
static void downloadParameters(CommonMemory& memory, mab::MD& md);

static void updateVelParameters(CommonMemory& memory);
static void updatePosParameters(CommonMemory& memory);
static void updateImpParameters(CommonMemory& memory);

// hardware thread loop
void candleLoop(CommonMemory& memory, std::atomic<bool>& isRunning);