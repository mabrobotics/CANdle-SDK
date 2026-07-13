#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "candle.hpp"
#include "MD.hpp"
#include <stdio.h>
#include <math.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

enum selectedDisplay
{
    position,
    velocity,
    torque
};

static bool show_demo_window  = false;
static bool systemON          = true;
static bool detectMD          = false;
static bool displayDetectedMD = false;
static bool testStarted       = false;

static int   monitorX, monitorY;
static float leftMenuBarWidth = 500.0f;
static float margin           = 10.f;
// Back menu settings
ImGuiWindowFlags flagsBackMenu = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

// Table settings
ImGuiTableFlags flagsTables = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

ImColor mabColor = ImColor::HSV(0.078f, 1.0f, 1.0f);

// MAB
std::vector<mab::MD> mdV;
mab::canId_t         chosenID;
mab::MDRegisters_S   registers;
mab::MDRegisters_S   registers2;

mab::MdMode_E currentMode = mab::MdMode_E::IDLE;

// MAXIMUM VALUES
static float maxVelocityClamp     = 0.0f;
static float maxPositionClamp     = 0.0f;
static float minPositionClamp     = 0.0f;
static float maxTorqueClamp       = 0.0f;
static float maxAccelerationClamp = 0.0f;
static float maxDecelerationClamp = 0.0f;

// TUNING GAINS
static float Kp_vel = 0.0f;
static float Ki_vel = 0.0f;
static float Kd_vel = 0.0f;

static float Kp_pos = 0.0f;
static float Ki_pos = 0.0f;
static float Kd_pos = 0.0f;

static float Kp_imp = 0.0f;
static float Kd_imp = 0.0f;

static float positionWindowSlider = 0.0f;
static float positionWindow       = 0.1f;
static float velocityWindowSlider = 0.0f;
static float velocityWindow       = 0.5f;

static float targetVelocitySlider     = 0.0f;
static float targetVelocity           = 0.0f;
static float targetPositionSlider     = 0.0f;
static float targetPosition           = 0.0f;
static float targetTorqueSlider       = 0.0f;
static float targetTorque             = 0.0f;
static float targetAccelerationSlider = 0.0f;
static float targetAcceleration       = 0.0f;
static float targetDecelrationSlider  = 0.0f;
static float targetDecelration        = 0.0f;
const float  step                     = 0.1f;
const float  step_fast                = 1.0f;

// functions
static void drawMenuTopBar(ImGuiIO& io);

static void drawMainMenu(ImGuiIO& io, mab::Candle* candle);
static void drawLeftMenuBar(ImGuiIO& io, mab::Candle* candle);

static void drawPIDtunerVelocity();
static void drawPIDtunerPosition();
static void drawPDtunerImpedance();

static void drawVelocityPlot(ImGuiIO& io, mab::Candle* candle);
static void drawPositionPlot(ImGuiIO& io, mab::Candle* candle);
static void drawTorquePlot(ImGuiIO& io, mab::Candle* candle);

static void drawSetTargetVelocity();
static void drawSetTargetPosition();
static void drawSetTargetTorque();
static void drawSetTargetAcceleration();
static void drawSetTargetDeceleration();
static void drawSetPositionWindow();
static void drawSetVelocityWindow();

static void drawTestButton(mab::Candle* candle);
static void drawEndTestButton(mab::Candle* candle);
static void drawDiscoverMDButton(mab::Candle* candle);
static void drawEnableMDButton(mab::Candle* candle);
static void drawDisableMDButton(mab::Candle* candle);
static void drawToggleButton(mab::Candle* candle);
static void drawSelectModeButton();

static void CenterText(const char* text);
const char* getModeName(mab::MdMode_E mode);
static bool drawBigInputFloat(
    const char* label, float* v, float step, float step_fast, const char* format);
static bool drawOrangeInputFloat(const char* label,
                                 float*      v,
                                 float       step      = 0.0f,
                                 float       step_fast = 0.0f,
                                 const char* format    = "%.3f");

static void testMD();
static void downloadParameters();
static void addMD100(mab::Candle* candle);