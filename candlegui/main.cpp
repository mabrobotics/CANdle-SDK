#define GL_SILENCE_DEPRECATION

#include "main.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// CUSTOM FUNCTIONALITY
static void drawMenuTopBar(ImGuiIO& io)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
            {
            }  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X"))
            {
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C"))
            {
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V"))
            {
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void drawPIDtunerVelocity(ImGuiIO& io)
{
    ImGui::Separator();
    CenterText("Velocity loop - PID tuning parameters");
    ImGui::Spacing();

    // Change this to whatever width fits your sidebar best
    float sliderWidth = leftMenuBar_width - 75.f;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Kp Vel", &Kp_vel, 0.0f, 1.0f);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Ki Vel", &Ki_vel, 0.0f, 1.0f);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Kd Vel", &Kd_vel, 0.0f, 1.0f);

    ImGui::PopStyleColor(4);
}

static void drawPIDtunerPosition(ImGuiIO& io)
{
    ImGui::Separator();
    CenterText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    // Change this to whatever width fits your sidebar best
    float sliderWidth = leftMenuBar_width - 75.f;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Kp Pos", &Kp_pos, 0.0f, 1.0f);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Ki Pos", &Ki_pos, 0.0f, 1.0f);

    ImGui::SetNextItemWidth(sliderWidth);
    ImGui::SliderFloat("Kd Pos", &Kd_pos, 0.0f, 1.0f);

    ImGui::PopStyleColor(4);
}

static void drawLeftMenuBar(ImGuiIO& io)
{
    const ImGuiViewport* leftMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(leftMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 windowSize = ImVec2(leftMenuBar_width, leftMenuViewport->WorkSize.y);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Tuning params!", nullptr, flagsBackMenu))
    {
        if (ImGui::BeginTabBar("##TabBar"))
        {
            if (ImGui::BeginTabItem("Main Menu"))
            {
                drawToggleButton();  // Switch button

                ImGui::ColorEdit3("clear color",
                                  (float*)&clear_color);  // Edit 3 floats representing a color

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / io.Framerate,
                            io.Framerate);

                drawPIDtunerVelocity(io);
                drawPIDtunerPosition(io);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Config"))
            {
                drawDiscoverMDButton();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

#include "implot.h"

static void drawPlot(ImGuiIO& io)
{
    const int buffer_size = 120;

    static float values[buffer_size]  = {};
    static float values2[buffer_size] = {};

    static int values_offset = 0;

    static float phase = 0.0f;

    if (refresh_time == 0.0)
        refresh_time = ImGui::GetTime();

    while (refresh_time < ImGui::GetTime())
    {
        values[values_offset]  = sinf(phase);
        values2[values_offset] = 2.0f * sinf(phase);

        values_offset = (values_offset + 1) % buffer_size;

        phase += 0.1f;
        refresh_time += 1.0f / 20.0f;
    }

    ImPlotSpec spec;
    spec.Offset = values_offset;

    if (ImPlot::BeginPlot("##SinePlot", ImVec2(-1, 250.0f)))
    {
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.5, 1.5, ImPlotCond_Always);

        ImPlot::PlotLine("Animated sin(t)", values, buffer_size, 1.0, 0.0, spec);

        ImPlot::EndPlot();
    }

    if (ImPlot::BeginPlot("##SinePlot2", ImVec2(-1, 250.0f)))
    {
        ImPlot::SetupAxisLimits(ImAxis_Y1, -2.5, 2.5, ImPlotCond_Always);

        ImPlot::PlotLine("Animated 2 sin(t)", values2, buffer_size, 1.0, 0.0, spec);

        ImPlot::EndPlot();
    }
}

static void drawMainMenu(ImGuiIO& io)
{
    // Size adjustment
    const ImGuiViewport* mainMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(mainMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 mainPos =
        ImVec2(mainMenuViewport->WorkPos.x + leftMenuBar_width, mainMenuViewport->WorkPos.y);
    ImGui::SetNextWindowPos(mainPos, ImGuiCond_Always);

    ImVec2 mainSize =
        ImVec2(mainMenuViewport->WorkSize.x - leftMenuBar_width, mainMenuViewport->WorkSize.y);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Main menu", nullptr, flagsBackMenu))
    {
        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Demo Window", &show_demo_window);
        ImGui::Checkbox("Another Window", &show_another_window);

        if (systemON)
        {
            drawPlot(io);
        }
    }

    ImGui::End();
}

static void drawToggleButton()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
    ImVec4 colorNormal, colorHovered, colorActive;

    if (systemON)
    {
        colorNormal  = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
        colorHovered = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);
        colorActive  = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
    }
    else
    {
        colorNormal  = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        colorHovered = ImVec4(0.3f, 0.9f, 0.3f, 1.0f);
        colorActive  = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
    }
    ImGui::PushStyleColor(ImGuiCol_Button, colorNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button(systemON ? "SYSTEM OFF" : "SYSTEM ON",
                      ImVec2(leftMenuBar_width - (margin * 2.0f), 80.0f)))
    {
        systemON = !systemON;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
}

static void drawDiscoverMDButton()
{
    ImGui::SetCursorPosX(margin);
    if (ImGui::Button(detectMD ? "SYSTEM OFF" : "SYSTEM ON",
                      ImVec2(leftMenuBar_width - (margin * 2.0f), 80.0f)))
    {
        detectMD = !detectMD;
    }
}

static void CenterText(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth   = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + generally GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + generally GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWmonitor*       primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode      = glfwGetVideoMode(primaryMonitor);

    GLFWwindow* window =
        glfwCreateWindow(videoMode->width, videoMode->height, "MABtuner app", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);
    glfwSetWindowPos(window, monitorX, monitorY);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    // ImGuiStyle& style = ImGui::GetStyle();
    // style.ScaleAllSizes(mainScale);        // Bake a fixed style scale. (until we have a solution
    // for dynamic style scaling, changing this requires resetting Style + calling this again)
    // style.FontScaleDpi = mainScale;        // Set initial font scale. (in docking branch: using
    // io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the
    // current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Candle setup
    // auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
    //                               mab::candleTypes::busTypes_t::USB);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawMenuTopBar(io);
        drawLeftMenuBar(io);
        drawMainMenu(io);

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        if (show_another_window)
        {
            ImGui::Begin(
                "Another Window",
                &show_another_window);  // Pass a pointer to our bool variable (the window will have
                                        // a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
