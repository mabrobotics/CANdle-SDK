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
static void drawMenuTopBar(CommonMemory& memory, ImGuiIO& io)
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

static void drawMainMenu(CommonMemory& memory, ImGuiIO& io)
{
    const ImGuiViewport* mainMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(mainMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 mainPos =
        ImVec2(mainMenuViewport->WorkPos.x + leftMenuBarWidth, mainMenuViewport->WorkPos.y);
    ImGui::SetNextWindowPos(mainPos, ImGuiCond_Always);

    ImVec2 mainSize =
        ImVec2(mainMenuViewport->WorkSize.x - leftMenuBarWidth, mainMenuViewport->WorkSize.y);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Main menu", nullptr, flagsBackMenu))
    {
        if (systemON)
        {
            drawVelocityPlot(memory, io);
            drawPositionPlot(memory, io);
            drawTorquePlot(memory, io);
        }
    }

    ImGui::End();
}

static void drawLeftMenuBar(CommonMemory& memory, ImGuiIO& io)
{
    const ImGuiViewport* leftMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(leftMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 windowSize = ImVec2(leftMenuBarWidth, leftMenuViewport->WorkSize.y);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Tuning params!", nullptr, flagsBackMenu))
    {
        if (ImGui::BeginTabBar("##TabBar"))
        {
            if (ImGui::BeginTabItem("Main Menu"))
            {
                drawToggleButton();

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / io.Framerate,
                            io.Framerate);

                ImGui::Separator();
                drawDiscoverMDButton(memory);

                drawSelectMDButton(memory);
                drawSelectModeButton(memory);

                ImGui::Separator();
                drawTestButton(memory);
                drawEndTestButton(memory);

                switch (memory.currentMode)
                {
                    case mab::MdMode_E::IDLE:
                        break;
                    case mab::MdMode_E::VELOCITY_PID:
                        drawPIDtunerVelocity();
                        drawSetTargetVelocity();
                        drawSetVelocityWindow();
                        break;
                    case mab::MdMode_E::POSITION_PID:
                        drawPIDtunerPosition();
                        drawSetTargetPosition();
                        drawSetPositionWindow();
                        break;
                    case mab::MdMode_E::IMPEDANCE:
                        drawPDtunerImpedance();
                        drawSetTargetPosition();
                        break;
                    case mab::MdMode_E::RAW_TORQUE:  // case unused
                        drawSetTargetTorque();
                        break;
                    case mab::MdMode_E::VELOCITY_PROFILE:
                        drawPIDtunerVelocity();
                        drawPIDtunerPosition();
                        drawSetTargetVelocity();
                        drawSetTargetPosition();
                        drawSetTargetAcceleration();
                        drawSetTargetDeceleration();
                        break;
                    case mab::MdMode_E::POSITION_PROFILE:
                        drawPIDtunerVelocity();
                        drawPIDtunerPosition();
                        drawSetTargetVelocity();
                        drawSetTargetPosition();
                        drawSetTargetAcceleration();
                        drawSetTargetDeceleration();
                        break;
                    default:
                        break;
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Config"))
            {
                drawParametersTable();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

static void drawPIDtunerVelocity()
{
    ImGui::Separator();
    CenterText("Velocity loop - PID tuning parameters");
    ImGui::Spacing();

    float sliderWidth = leftMenuBarWidth - 75.f;

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kp Vel", &Kp_velSlider, 0.01f, 0.1f))
    {
        Kp_velSlider = std::clamp(Kp_velSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Ki Vel", &Ki_velSlider, 0.01f, 0.1f))
    {
        Ki_velSlider = std::clamp(Ki_velSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Vel", &Kd_velSlider, 0.01f, 0.1f))
    {
        Kd_velSlider = std::clamp(Kd_velSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Windup Vel", &integralMax_velSlider, 0.01f, 0.1f))
    {
        integralMax_velSlider = std::clamp(integralMax_velSlider, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Velocity Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_velSlider          = 0.0f;
        Ki_velSlider          = 0.0f;
        Kd_velSlider          = 0.0f;
        integralMax_velSlider = 0.0f;
    }
}

static void drawPIDtunerPosition()
{
    ImGui::Separator();
    CenterText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    float sliderWidth = leftMenuBarWidth - 75.f;

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kp Pos", &Kp_posSlider, 0.01f, 0.1f))
    {
        Kp_posSlider = std::clamp(Kp_posSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Ki Pos", &Ki_posSlider, 0.01f, 0.1f))
    {
        Ki_posSlider = std::clamp(Ki_posSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Pos", &Kd_posSlider, 0.01f, 0.1f))
    {
        Kd_posSlider = std::clamp(Kd_posSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Windup Pos", &integralMax_posSlider, 0.01f, 0.1f))
    {
        integralMax_posSlider = std::clamp(integralMax_posSlider, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Position Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_posSlider          = 0.0f;
        Ki_posSlider          = 0.0f;
        Kd_posSlider          = 0.0f;
        integralMax_posSlider = 0.0f;
    }
}

static void drawPDtunerImpedance()
{
    ImGui::Separator();
    CenterText("Impedance PD tuner");
    ImGui::Spacing();

    float sliderWidth = leftMenuBarWidth - 75.f;

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kp Imp", &Kp_impSlider, 0.01f, 0.1f))
    {
        Kp_impSlider = std::clamp(Kp_impSlider, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Imp", &Kd_impSlider, 0.01f, 0.1f))
    {
        Kd_impSlider = std::clamp(Kd_impSlider, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Impedance Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_impSlider = 0.0f;
        Kd_impSlider = 0.0f;
    }
}

static void drawVelocityPlot(CommonMemory& memory, ImGuiIO& io)
{
    static const int buffer_size = 10000;

    static float timeValues[buffer_size]        = {0};
    static float measurementValues[buffer_size] = {0};
    static float targetValues[buffer_size]      = {0};

    static int    offset       = 0;
    static float  t            = 0.0f;
    static double refresh_time = 0.0f;

    static bool  lastTestStarted    = false;
    static bool  isTargetAchieved   = false;
    static float timeInTargetWindow = 0.0f;

    static float maxMeasured = 0.0f;
    static float minMeasured = 0.0f;

    if (memory.testStarted && !lastTestStarted)
    {
        t                  = 0.0f;
        offset             = 0.0f;
        isTargetAchieved   = false;
        timeInTargetWindow = 0.0f;
        refresh_time       = ImGui::GetTime();

        minMeasured = (memory.targetVelocity < 0.0f) ? memory.targetVelocity : 0.0f;
        maxMeasured = (memory.targetVelocity > 0.0f) ? memory.targetVelocity : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = memory.testStarted;

    float currentVel = memory.currentVelMeasured;

    if (memory.testStarted && !isTargetAchieved)
    {
        if (currentVel > maxMeasured)
            maxMeasured = currentVel;
        if (currentVel < minMeasured)
            minMeasured = currentVel;

        bool inTargetWindow = std::abs(currentVel - memory.targetVelocity) <= velocityWindow;

        if (inTargetWindow)
        {
            timeInTargetWindow += io.DeltaTime;

            if (timeInTargetWindow >= 0.25f)
            {
                isTargetAchieved   = true;
                memory.testStarted = false;
            }
        }
        else
        {
            timeInTargetWindow = 0.0f;
        }

        if (refresh_time == 0.0f)
            refresh_time = ImGui::GetTime();

        while (refresh_time < ImGui::GetTime())
        {
            t += 1.0f / 60.0f;

            timeValues[offset]        = t;
            measurementValues[offset] = currentVel;
            targetValues[offset]      = memory.targetVelocity;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##VelocityPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 3)))
    {
        ImPlotCond limitCondition = memory.testStarted ? ImPlotCond_Always : ImPlotCond_Once;

        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0f, t, limitCondition);

        float bottomY = (minMeasured < 0.0f) ? (minMeasured - margin) : 0.0f;
        float topY    = (maxMeasured > 0.0f) ? (maxMeasured + margin) : 0.0f;

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = margin;
            bottomY = -margin;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, limitCondition);

        ImPlot::PlotLine("Velocity(t)", timeValues, measurementValues, buffer_size, spec);

        if (memory.currentMode == mab::MdMode_E::VELOCITY_PID ||
            memory.currentMode == mab::MdMode_E::VELOCITY_PROFILE ||
            memory.currentMode == mab::MdMode_E::POSITION_PROFILE)
            ImPlot::PlotStairs("Target Velocity", timeValues, targetValues, buffer_size, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawPositionPlot(CommonMemory& memory, ImGuiIO& io)
{
    static const int buffer_size = 10000;

    static float timeValues[buffer_size]        = {0};
    static float measurementValues[buffer_size] = {0};
    static float targetValues[buffer_size]      = {0};

    static int    offset       = 0;
    static float  t            = 0.0f;
    static double refresh_time = 0.0f;

    static bool  lastTestStarted    = false;
    static bool  isTargetAchieved   = false;
    static float timeInTargetWindow = 0.0f;

    static float maxMeasured = 0.0f;
    static float minMeasured = 0.0f;

    if (memory.testStarted && !lastTestStarted)
    {
        t                  = 0.0f;
        offset             = 0.0f;
        isTargetAchieved   = false;
        timeInTargetWindow = 0.0f;
        refresh_time       = ImGui::GetTime();

        minMeasured = (memory.targetPosition < 0.0f) ? memory.targetPosition : 0.0f;
        maxMeasured = (memory.targetPosition > 0.0f) ? memory.targetPosition : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = memory.testStarted;

    float currentPos = memory.currentPosMeasured;

    if (memory.testStarted && !isTargetAchieved)
    {
        if (currentPos > maxMeasured)
            maxMeasured = currentPos;
        if (currentPos < minMeasured)
            minMeasured = currentPos;

        bool inTargetWindow = std::abs(currentPos - memory.targetPosition) <= positionWindow;

        if (inTargetWindow)
        {
            timeInTargetWindow += io.DeltaTime;

            if (timeInTargetWindow >= 0.25f)
            {
                isTargetAchieved   = true;
                memory.testStarted = false;
            }
        }
        else
        {
            timeInTargetWindow = 0.0f;
        }

        if (refresh_time == 0.0f)
            refresh_time = ImGui::GetTime();

        while (refresh_time < ImGui::GetTime())
        {
            t += 1.0f / 60.0f;

            timeValues[offset]        = t;
            measurementValues[offset] = currentPos;
            targetValues[offset]      = memory.targetPosition;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##PositionPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 2)))
    {
        ImPlotCond limitCondition = memory.testStarted ? ImPlotCond_Always : ImPlotCond_Once;

        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0f, t, limitCondition);

        float bottomY = (minMeasured < 0.0f) ? (minMeasured - margin) : 0.0f;
        float topY    = (maxMeasured > 0.0f) ? (maxMeasured + margin) : 0.0f;

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = margin;
            bottomY = -margin;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, limitCondition);

        ImPlot::PlotLine("Position(t)", timeValues, measurementValues, buffer_size, spec);

        if (memory.currentMode == mab::MdMode_E::POSITION_PID ||
            memory.currentMode == mab::MdMode_E::POSITION_PROFILE ||
            memory.currentMode == mab::MdMode_E::IMPEDANCE ||
            memory.currentMode == mab::MdMode_E::VELOCITY_PROFILE)
            ImPlot::PlotStairs("Target Position", timeValues, targetValues, buffer_size, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawTorquePlot(CommonMemory& memory, ImGuiIO& io)
{
    static const int buffer_size = 10000;

    static float timeValues[buffer_size]        = {0};
    static float measurementValues[buffer_size] = {0};
    static float targetValues[buffer_size]      = {0};

    static int    offset       = 0;
    static float  t            = 0.0f;
    static double refresh_time = 0.0f;

    static bool lastTestStarted = false;

    static float maxMeasured = 0.0f;
    static float minMeasured = 0.0f;

    if (memory.testStarted && !lastTestStarted && memory.currentMode != mab::MdMode_E::IDLE)
    {
        t            = 0.0f;
        offset       = 0.0f;
        refresh_time = ImGui::GetTime();

        minMeasured = (memory.targetTorque < 0.0f) ? memory.targetTorque : 0.0f;
        maxMeasured = (memory.targetTorque > 0.0f) ? memory.targetTorque : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = memory.testStarted;

    float currentTorque = memory.currentTorqueMeasured;

    if (memory.testStarted && memory.currentMode != mab::MdMode_E::IDLE)
    {
        if (currentTorque > maxMeasured)
            maxMeasured = currentTorque;
        if (currentTorque < minMeasured)
            minMeasured = currentTorque;

        if (refresh_time == 0.0f)
            refresh_time = ImGui::GetTime();

        while (refresh_time < ImGui::GetTime())
        {
            t += 1.0f / 60.0f;

            timeValues[offset]        = t;
            measurementValues[offset] = currentTorque;
            targetValues[offset]      = memory.targetTorque;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##TorquePlot", ImVec2(-1, ImGui::GetContentRegionAvail().y)))
    {
        ImPlotCond limitCondition = memory.testStarted ? ImPlotCond_Always : ImPlotCond_Once;

        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0f, t, limitCondition);

        float bottomY = (minMeasured < 0.0f) ? (minMeasured - margin) : 0.0f;
        float topY    = (maxMeasured > 0.0f) ? (maxMeasured + margin) : 0.0f;

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = margin;
            bottomY = -margin;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, limitCondition);

        ImPlot::PlotLine("Torque(t)", timeValues, measurementValues, buffer_size, spec);

        if (memory.currentMode == mab::MdMode_E::IMPEDANCE)
            ImPlot::PlotStairs("Target Torque", timeValues, targetValues, buffer_size, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawSetTargetVelocity()
{
    ImGui::Separator();
    if (drawBigInputFloat("Target Velocity", &targetVelocitySlider, 1.0f, 2.0f, "%.2f"))
    {
        targetVelocitySlider =
            std::clamp(targetVelocitySlider, -maxVelocityClamp, maxVelocityClamp);
    }
}

static void drawSetTargetPosition()
{
    ImGui::Separator();
    if (drawBigInputFloat("Target Position", &targetPositionSlider, 1.0f, 2.0f, "%.2f"))
    {
        targetPositionSlider = std::clamp(targetPositionSlider, minPositionClamp, maxPositionClamp);
    }
}

static void drawSetTargetTorque()
{
    ImGui::Separator();
    if (drawBigInputFloat("Target Torque", &targetTorqueSlider, 1.0f, 2.0f, "%.2f"))
    {
        targetTorqueSlider = std::clamp(targetTorqueSlider, -maxTorqueClamp, maxTorqueClamp);
    }
}

static void drawSetTargetAcceleration()
{
    ImGui::Separator();
    if (drawBigInputFloat("Acceleration", &targetAccelerationSlider, step, step_fast, "%.2f"))
    {
        targetAccelerationSlider = std::clamp(targetAccelerationSlider, 0.0f, maxAccelerationClamp);
    }
}

static void drawSetTargetDeceleration()
{
    ImGui::Separator();
    if (drawBigInputFloat("Deceleration", &targetDecelrationSlider, step, step_fast, "%.2f"))
    {
        targetDecelrationSlider = std::clamp(targetDecelrationSlider, 0.0f, maxDecelerationClamp);
    }
}

static void drawSetPositionWindow()
{
    ImGui::Separator();
    drawBigInputFloat("Position Window", &positionWindowSlider, step, step_fast, "%.2f");
}

static void drawSetVelocityWindow()
{
    ImGui::Separator();
    drawBigInputFloat("Velocity Window", &velocityWindowSlider, step, step_fast, "%.2f");
}

static void drawTestButton(CommonMemory& memory)
{
    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::BeginDisabled();
    }
    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        {
            std::lock_guard<std::mutex> lock(memory.mtx);
            memory.testStarted           = true;
            memory.updateParametersTest  = true;
            memory.currentPosMeasured    = 0.0f;
            memory.currentVelMeasured    = 0.0f;
            memory.currentTorqueMeasured = 0.0f;
        }
        switch (memory.currentMode)
        {
            case mab::MdMode_E::IDLE:
                break;
            case mab::MdMode_E::VELOCITY_PID:
                memory.targetVelocity = targetVelocitySlider;
                memory.targetPosition = 0.0f;
                positionWindow        = 0.01;
                velocityWindow        = velocityWindowSlider;
                updateVelParameters(memory);
                break;
            case mab::MdMode_E::POSITION_PID:
                memory.targetPosition = targetPositionSlider;
                memory.targetVelocity = 0.0f;
                velocityWindow        = 0.01;
                positionWindow        = positionWindowSlider;
                updatePosParameters(memory);
                break;
            case mab::MdMode_E::IMPEDANCE:
                memory.targetPosition = targetPositionSlider;
                updateImpParameters(memory);
                break;
            case mab::MdMode_E::RAW_TORQUE:  // case unused
                break;
            case mab::MdMode_E::VELOCITY_PROFILE:
                memory.targetPosition     = targetPositionSlider;
                memory.targetVelocity     = targetVelocitySlider;
                memory.targetAcceleration = targetAccelerationSlider;
                memory.targetDecelration  = targetDecelrationSlider;
                positionWindow            = 0.01;
                velocityWindow            = velocityWindowSlider;
                updateVelParameters(memory);
                updatePosParameters(memory);
                break;
            case mab::MdMode_E::POSITION_PROFILE:
                memory.targetPosition     = targetPositionSlider;
                memory.targetVelocity     = targetVelocitySlider;
                memory.targetAcceleration = targetAccelerationSlider;
                memory.targetDecelration  = targetDecelrationSlider;
                velocityWindow            = 0.01;
                positionWindow            = positionWindowSlider;
                updateVelParameters(memory);
                updatePosParameters(memory);
                break;
        }
    }
    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::EndDisabled();
    }
}

static void drawEndTestButton(CommonMemory& memory)
{
    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::BeginDisabled();
    }
    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("End test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        memory.testStarted = false;
    }
    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::EndDisabled();
    }
}

static void drawDiscoverMDButton(CommonMemory& memory)
{
    ImGui::SetCursorPosX(margin);
    if (discoverOngoing)
    {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Discover MD", ImVec2(leftMenuBarWidth - (margin * 2.0f), 20.0f)))
    {
        memory.buttonDiscoverMdPressed = true;
    }

    if (discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

static void drawParametersTable()
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    const int numberOfColumns = 2;

    ImGui::Text("Downloaded config");

    if (ImGui::BeginTable("ParamTable", numberOfColumns, flags))
    {
        ImGui::TableSetupColumn("Parameters name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp Velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kp_vel);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki Velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Ki_vel);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd Velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kd_vel);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Windup Velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.integralMax_vel);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp Position");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kp_pos);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki Position");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Ki_pos);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd Position");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kd_pos);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Windup Position");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.integralMax_pos);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp Impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kp_imp);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd Impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", tableData.Kd_imp);

        ImGui::EndTable();
    }
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
                      ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        systemON = !systemON;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
}

static void drawSelectModeButton(CommonMemory& memory)
{
    ImGui::Separator();

    if (!selectedMD)
    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        memory.currentMode = mab::MdMode_E::IDLE;
        ImGui::BeginDisabled();
    }

    if (ImGui::BeginCombo("MD Motion Mode", getModeName(memory.currentMode)))
    {
        if (ImGui::Selectable("None", memory.currentMode == mab::MdMode_E::IDLE))
        {
            memory.currentMode = mab::MdMode_E::IDLE;
            selectedMode       = true;
        }
        if (ImGui::Selectable("Velocity PID", memory.currentMode == mab::MdMode_E::VELOCITY_PID))
        {
            memory.currentMode = mab::MdMode_E::VELOCITY_PID;
            selectedMode       = true;
        }
        if (ImGui::Selectable("Position PID", memory.currentMode == mab::MdMode_E::POSITION_PID))
        {
            memory.currentMode = mab::MdMode_E::POSITION_PID;
            selectedMode       = true;
        }
        if (ImGui::Selectable("Impedance PD", memory.currentMode == mab::MdMode_E::IMPEDANCE))
        {
            memory.currentMode = mab::MdMode_E::IMPEDANCE;
            selectedMode       = true;
        }
        if (ImGui::Selectable("Velocity Profile",
                              memory.currentMode == mab::MdMode_E::VELOCITY_PROFILE))
        {
            memory.currentMode = mab::MdMode_E::VELOCITY_PROFILE;
            selectedMode       = true;
        }
        if (ImGui::Selectable("Position Profile",
                              memory.currentMode == mab::MdMode_E::POSITION_PROFILE))
        {
            memory.currentMode = mab::MdMode_E::POSITION_PROFILE;
            selectedMode       = true;
        }

        ImGui::EndCombo();
    }

    if (!selectedMD)
    {
        ImGui::EndDisabled();
    }
}

static void drawSelectMDButton(CommonMemory& memory)
{
    ImGui::Separator();

    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID = 0;

    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        mdIDs    = memory.mdIDs;
        chosenID = memory.chosenID;
    }

    if (discoverOngoing)
    {
        selectedMD   = false;
        selectedMode = false;
    }

    if (discoverOngoing)
    {
        double time       = ImGui::GetTime();
        int    dotCounter = (int)(time * 2.5) % 4;

        chosenIDstr = "Discover ongoing" + std::string("...").substr(0, dotCounter);
        ImGui::BeginDisabled();
    }
    else if (mdIDs.empty())
    {
        chosenIDstr = "No MDs available.";
        selectedMD  = false;
        ImGui::BeginDisabled();
    }
    else if (!selectedMD)
        chosenIDstr = "Select Your MD";

    if (ImGui::BeginCombo("MD Select", chosenIDstr.c_str()))
    {
        for (const auto& id : mdIDs)
        {
            std::string chosenIDname = "MD" + std::to_string(int((id)));

            bool selectedID = (chosenID == id);

            if (ImGui::Selectable(chosenIDname.c_str()))
            {
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.chosenID     = id;
                memory.selectedMDid = true;
                chosenIDstr         = chosenIDname;
                selectedMD          = true;
            }
            if (selectedID)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
    else if (discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

static void testMD(CommonMemory& memory, mab::MD& md)
{
    switch (memory.currentMode)
    {
        case mab::MdMode_E::IDLE:
            break;
        case mab::MdMode_E::VELOCITY_PID:
            md.setTargetVelocity(memory.targetVelocity);
            break;
        case mab::MdMode_E::POSITION_PID:
            md.setTargetPosition(memory.targetPosition);
            break;
        case mab::MdMode_E::IMPEDANCE:
            md.setTargetPosition(memory.targetPosition);
            break;
        case mab::MdMode_E::RAW_TORQUE:  // case unused
            memory.targetTorque = targetTorqueSlider;
            md.setTargetTorque(memory.targetTorque);
            break;
        case mab::MdMode_E::VELOCITY_PROFILE:
            md.setTargetPosition(memory.targetPosition);
            md.setTargetVelocity(memory.targetVelocity);
            md.setProfileAcceleration(memory.targetAcceleration);
            md.setProfileDeceleration(memory.targetDecelration);
            break;
        case mab::MdMode_E::POSITION_PROFILE:
            md.setTargetPosition(memory.targetPosition);
            md.setTargetVelocity(memory.targetVelocity);
            md.setProfileAcceleration(memory.targetAcceleration);
            md.setProfileDeceleration(memory.targetDecelration);
            break;
        default:
            break;
    }
}

static void CenterText(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth   = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}

const char* getModeName(mab::MdMode_E mode)
{
    switch (mode)
    {
        case mab::MdMode_E::IDLE:
            return "None";
        case mab::MdMode_E::VELOCITY_PID:
            return "Velocity PID";
        case mab::MdMode_E::POSITION_PID:
            return "Position PID";
        case mab::MdMode_E::IMPEDANCE:
            return "Impedance PD";
        case mab::MdMode_E::RAW_TORQUE:  // case unused
            return "Raw Torque";
        case mab::MdMode_E::VELOCITY_PROFILE:
            return "Velocity Profile";
        case mab::MdMode_E::POSITION_PROFILE:
            return "Position Profile";
        default:
            return "Unknown mode";
    }
}

static bool drawBigInputFloat(
    const char* label, float* v, float step, float step_fast, const char* format)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));

    bool valueChanged =
        ImGui::InputFloat(label, v, step, step_fast, format, ImGuiInputTextFlags_None);

    ImGui::PopStyleVar();
    ImGui::SetWindowFontScale(1.0f);

    return valueChanged;
}

static bool drawOrangeInputFloat(
    const char* label, float* v, float step, float step_fast, const char* format)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    bool valueChanged = ImGui::InputFloat(label, v, step, step_fast, format);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    return valueChanged;
}

static void downloadParameters(CommonMemory& memory, mab::MD& md)
{
    mab::MD::Error_t err = md.readRegisters(registers.motorImpPidKp,
                                            registers.motorImpPidKd,
                                            registers.motorVelPidKp,
                                            registers.motorVelPidKi,
                                            registers.motorVelPidKd,
                                            registers.motorPosPidKp,
                                            registers.motorPosPidKi,
                                            registers.motorPosPidKd,
                                            registers.positionWindow,
                                            registers.velocityWindow);

    mab::MD::Error_t err2 = md.readRegisters(registers.maxVelocity,
                                             registers.positionLimitMax,
                                             registers.positionLimitMin,
                                             registers.maxTorque,
                                             registers.maxAcceleration,
                                             registers.maxDeceleration,
                                             registers.motorVelPidWindup,
                                             registers.motorPosPidWindup);

    if (err != mab::MD::Error_t::OK && err2 != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(memory.mtx);
            memory.Kp_vel          = float(registers.motorVelPidKp.value);
            memory.Ki_vel          = float(registers.motorVelPidKi.value);
            memory.Kd_vel          = float(registers.motorVelPidKd.value);
            memory.integralMax_vel = float(registers.motorVelPidWindup.value);

            memory.Kp_pos          = float(registers.motorPosPidKp.value);
            memory.Ki_pos          = float(registers.motorPosPidKi.value);
            memory.Kd_pos          = float(registers.motorPosPidKd.value);
            memory.integralMax_pos = float(registers.motorPosPidWindup.value);

            memory.Kp_imp = float(registers.motorImpPidKp.value);
            memory.Kd_imp = float(registers.motorImpPidKd.value);
        }

        positionWindowSlider = float(registers.positionWindow.value);
        velocityWindowSlider = float(registers.velocityWindow.value);

        maxVelocityClamp     = float(registers.maxVelocity.value);
        maxPositionClamp     = float(registers.positionLimitMax.value);
        minPositionClamp     = float(registers.positionLimitMin.value);
        maxTorqueClamp       = float(registers.maxTorque.value);
        maxAccelerationClamp = float(registers.maxAcceleration.value);
        maxDecelerationClamp = float(registers.maxDeceleration.value);

        // Write them on init to sliders
        Kp_velSlider          = memory.Kp_vel;
        Ki_velSlider          = memory.Ki_vel;
        Kd_velSlider          = memory.Kd_vel;
        integralMax_velSlider = memory.integralMax_vel;

        Kp_posSlider          = memory.Kp_pos;
        Ki_posSlider          = memory.Ki_pos;
        Kd_posSlider          = memory.Kd_pos;
        integralMax_posSlider = memory.integralMax_pos;

        Kp_impSlider = memory.Kp_imp;
        Kd_impSlider = memory.Kd_imp;

        // Write them on into Table
        if (memory.firstDownload)
        {
            memory.firstDownload      = false;
            tableData.Kp_vel          = memory.Kp_vel;
            tableData.Ki_vel          = memory.Ki_vel;
            tableData.Kd_vel          = memory.Kd_vel;
            tableData.integralMax_vel = memory.integralMax_vel;

            tableData.Kp_pos          = memory.Kp_pos;
            tableData.Ki_pos          = memory.Ki_pos;
            tableData.Kd_pos          = memory.Kd_pos;
            tableData.integralMax_pos = memory.integralMax_pos;

            tableData.Kp_imp = memory.Kp_imp;
            tableData.Kd_imp = memory.Kd_imp;
        }
    }
}

static void updateVelParameters(CommonMemory& memory)
{
    memory.Kp_vel          = Kp_velSlider;
    memory.Ki_vel          = Ki_velSlider;
    memory.Kd_vel          = Kd_velSlider;
    memory.integralMax_vel = integralMax_velSlider;
}

static void updatePosParameters(CommonMemory& memory)
{
    memory.Kp_pos          = Kp_posSlider;
    memory.Ki_pos          = Ki_posSlider;
    memory.Kd_pos          = Kd_posSlider;
    memory.integralMax_pos = integralMax_posSlider;
}

static void updateImpParameters(CommonMemory& memory)
{
    memory.Kp_imp = Kp_impSlider;
    memory.Kd_imp = Kd_impSlider;
}

void candleLoop(CommonMemory& memory, std::atomic<bool>& isRunning)
{
    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);

    std::vector<mab::MD> mdV;

    while (isRunning)
    {
        bool          testStarted             = false;
        bool          updateParametersTest    = false;
        bool          buttonDiscoverMdPressed = false;
        bool          selectedMDid            = false;
        mab::MdMode_E currentMode             = mab::MdMode_E::IDLE;
        mab::canId_t  chosenID                = 0;

        {
            std::lock_guard<std::mutex> lock(memory.mtx);
            testStarted             = memory.testStarted;
            updateParametersTest    = memory.updateParametersTest;
            buttonDiscoverMdPressed = memory.buttonDiscoverMdPressed;
            selectedMDid            = memory.selectedMDid;
            currentMode             = memory.currentMode;
            chosenID                = memory.chosenID;

            memory.updateParametersTest    = false;
            memory.buttonDiscoverMdPressed = false;
            memory.selectedMDid            = false;
        }

        for (auto& md : mdV)
        {
            if (md.m_canId == chosenID)
            {
                if (selectedMDid)
                    downloadParameters(memory, md);
                if (updateParametersTest)
                {
                    md.zero();  // ZEROING FOR SAFETY TODO
                    if (md.setMotionMode(currentMode) != mab::MD::Error_t::OK)
                    {
                        std::cout << "MD mode setting failed \n";
                    }

                    switch (currentMode)
                    {
                        case mab::MdMode_E::IDLE:
                            break;
                        case mab::MdMode_E::VELOCITY_PID:
                            md.setVelocityPIDparam(memory.Kp_vel,
                                                   memory.Ki_vel,
                                                   memory.Kd_vel,
                                                   memory.integralMax_vel);
                            break;
                        case mab::MdMode_E::POSITION_PID:
                            md.setPositionPIDparam(memory.Kp_pos,
                                                   memory.Ki_pos,
                                                   memory.Kd_pos,
                                                   memory.integralMax_pos);
                            break;
                        case mab::MdMode_E::IMPEDANCE:
                            md.setImpedanceParams(memory.Kp_imp, memory.Kd_imp);
                            break;
                        case mab::MdMode_E::RAW_TORQUE:  // case unused
                            break;
                        case mab::MdMode_E::VELOCITY_PROFILE:
                            md.setVelocityPIDparam(memory.Kp_vel,
                                                   memory.Ki_vel,
                                                   memory.Kd_vel,
                                                   memory.integralMax_vel);
                            md.setPositionPIDparam(memory.Kp_pos,
                                                   memory.Ki_pos,
                                                   memory.Kd_pos,
                                                   memory.integralMax_pos);
                            break;
                        case mab::MdMode_E::POSITION_PROFILE:
                            md.setVelocityPIDparam(memory.Kp_vel,
                                                   memory.Ki_vel,
                                                   memory.Kd_vel,
                                                   memory.integralMax_vel);
                            md.setPositionPIDparam(memory.Kp_pos,
                                                   memory.Ki_pos,
                                                   memory.Kd_pos,
                                                   memory.integralMax_pos);
                            break;
                        default:
                            break;
                    }

                    md.enable();
                }

                if (testStarted && currentMode != mab::MdMode_E::IDLE)
                {
                    testMD(memory, md);

                    auto localVel = md.getVelocity().first;
                    auto localPos = md.getPosition().first;
                    auto localTrq = md.getTorque().first;

                    {
                        std::lock_guard<std::mutex> lock(memory.mtx);
                        memory.currentVelMeasured    = localVel;
                        memory.currentPosMeasured    = localPos;
                        memory.currentTorqueMeasured = localTrq;
                    }
                }
            }
        }

        if (buttonDiscoverMdPressed)
        {
            discoverOngoing = true;
            mdV.clear();
            for (const auto& id : mab::MD::discoverMDs(candle))
            {
                mdV.emplace_back(id, candle);
                mdV.back().init();
            }

            {
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.mdIDs.clear();
                for (const auto& id : mdV)
                {
                    memory.mdIDs.push_back(id.m_canId);
                }
            }
            discoverOngoing = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    mab::detachCandle(candle);
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
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Common memory init
    CommonMemory m_common;

    std::atomic<bool> isRunning{true};

    // Initialization ofo candlehardware thread
    std::thread hardware(candleLoop, std::ref(m_common), std::ref(isRunning));

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

        drawMenuTopBar(m_common, io);
        drawLeftMenuBar(m_common, io);
        drawMainMenu(m_common, io);

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
    isRunning = false;
    hardware.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
