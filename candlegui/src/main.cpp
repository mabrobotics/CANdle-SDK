#define GL_SILENCE_DEPRECATION
#include "GUI.hpp"
#include "HardwareCandle.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    std::string fontLocalPath  = "fonts/Roboto.ttf";
    std::string fontSystemPath = "/usr/share/mdgui/fonts/Roboto.ttf";
    std::string fontFinalPath  = "";

    if (std::filesystem::exists(fontLocalPath))
    {
        fontFinalPath = fontLocalPath;
    }
    else if (std::filesystem::exists(fontSystemPath))
    {
        fontFinalPath = fontSystemPath;
    }

    if (!fontFinalPath.empty())
    {
        io.Fonts->AddFontFromFileTTF(fontFinalPath.c_str(), 14.0f);
    }
    else
    {
        io.Fonts->AddFontDefault();
    }

    auto m_common = std::make_shared<commonMemory_S>();

    std::atomic<bool> isRunning{true};

    GraphicInterface interface(m_common, io);

    // Initialization of candlehardware thread
    HardwareCandle hardwareCandle(m_common);
    hardwareCandle.init();
    std::thread hardware(&HardwareCandle::candleLoop, &hardwareCandle, std::ref(isRunning));

    interface.init();

    // MAIN LOOP
    interface.loop();

    // Cleanup
    interface.close();
    isRunning = false;
    hardware.join();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
