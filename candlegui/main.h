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

static bool show_demo_window    = false;
static bool show_another_window = false;
static bool systemON            = false;

static bool detectMD          = false;
static bool displayDetectedMD = false;

static double refresh_time = 0.0;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

ImColor mabColor = ImColor::HSV(0.078f, 1.0f, 1.0f);

// MAB
std::vector<mab::MD> mdV;

mab::canId_t chosenID;

float Kp_vel = 0.0f;
float Ki_vel = 0.0f;
float Kd_vel = 0.0f;

float Kp_pos = 0.0f;
float Ki_pos = 0.0f;
float Kd_pos = 0.0f;

float Velocity = 0.0f;

float targetVelocity = 0.0f;

float leftMenuBar_width = 500.0f;
float margin            = 10.f;

int monitorX, monitorY;

// Back menu settings
ImGuiWindowFlags flagsBackMenu = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

// Table settings
ImGuiTableFlags flagsTables = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

// functions
static void drawMenuTopBar(ImGuiIO& io);

static void drawVelocityPlot(ImGuiIO& io, mab::Candle* candle);
static void drawPositionPlot(ImGuiIO& io, mab::Candle* candle);
static void drawTorquePlot(ImGuiIO& io, mab::Candle* candle);

static void drawMainMenu(ImGuiIO& io, mab::Candle* candle);

static void drawPIDtunerVelocity(ImGuiIO& io);
static void drawPIDtunerPosition(ImGuiIO& io);
static void drawLeftMenuBar(ImGuiIO& io, mab::Candle* candle);

static void drawDiscoverMDButton(mab::Candle* candle);
static void drawEnableMDButton(mab::Candle* candle);
static void drawDisableMDButton(mab::Candle* candle);
static void CenterText(const char* text);
static void drawToggleButton(mab::Candle* candle);