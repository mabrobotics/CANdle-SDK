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

    ImVec2 windowSize = ImVec2(leftMenuBarWidth, leftMenuViewport->WorkSize.y - testMenuBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Main Menu", nullptr, flagsBackMenu))
    {
        if (ImGui::BeginTabBar("##TabBar"))
        {
            if (ImGui::BeginTabItem("Main Menu"))
            {
                drawToggleButton();

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / io.Framerate,
                            io.Framerate);

                if (memory.testOngoing)
                {
                    ImGui::BeginDisabled();
                }
                drawDiscoverMDButton(memory);
                drawSelectMDButton(memory);
                drawSelectModeButton(memory);

                switch (memory.currentMode)
                {
                    case mab::MdMode_E::IDLE:
                        break;
                    case mab::MdMode_E::VELOCITY_PID:
                        drawParametersVelocity(memory);
                        drawSetTargetVelocity();
                        drawSetVelocityWindow();
                        break;
                    case mab::MdMode_E::POSITION_PID:
                        drawParametersPosition(memory);
                        drawSetTargetPosition();
                        drawSetPositionWindow();
                        break;
                    case mab::MdMode_E::IMPEDANCE:
                        drawParametersImpedance(memory);
                        drawSetTargetPosition();
                        break;
                    case mab::MdMode_E::RAW_TORQUE:  // case unused
                        drawSetTargetTorque();
                        break;
                    case mab::MdMode_E::VELOCITY_PROFILE:
                        drawParametersVelocity(memory);
                        drawParametersPosition(memory);
                        drawSetTargetVelocity();
                        drawSetTargetPosition();
                        drawSetTargetAcceleration();
                        drawSetTargetDeceleration();
                        break;
                    case mab::MdMode_E::POSITION_PROFILE:
                        drawParametersVelocity(memory);
                        drawParametersPosition(memory);
                        drawSetTargetVelocity();
                        drawSetTargetPosition();
                        drawSetTargetAcceleration();
                        drawSetTargetDeceleration();
                        break;
                    default:
                        break;
                }
                if (memory.testOngoing)
                {
                    ImGui::EndDisabled();
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

static void drawTestMenuBar(CommonMemory& memory, ImGuiIO& io)
{
    const ImGuiViewport* testMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(testMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 testPos =
        ImVec2(testMenuViewport->WorkPos.x,
               testMenuViewport->WorkPos.y + testMenuViewport->WorkSize.y - testMenuBarHeight);
    ImGui::SetNextWindowPos(testPos, ImGuiCond_Always);

    ImVec2 windowSize = ImVec2(leftMenuBarWidth, testMenuBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Test menu", nullptr, flagsBackMenu))
    {
        drawTestButton(memory);
        drawEndTestButton(memory);
    }
    ImGui::End();
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
                isTargetAchieved = true;
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.testStarted = false;
                memory.testOngoing = false;
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
            t += 1.0f / refreshRate;

            timeValues[offset]        = t;
            measurementValues[offset] = currentVel;
            targetValues[offset]      = memory.targetVelocity;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / refreshRate;
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
                memory.testOngoing = false;
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
            t += 1.0f / refreshRate;

            timeValues[offset]        = t;
            measurementValues[offset] = currentPos;
            targetValues[offset]      = memory.targetPosition;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / refreshRate;
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
            t += 1.0f / refreshRate;

            timeValues[offset]        = t;
            measurementValues[offset] = currentTorque;
            targetValues[offset]      = memory.targetTorque;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / refreshRate;
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
    else if (memory.testOngoing)
    {
        ImGui::BeginDisabled();
    }

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 40.0f)))
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
    else if (memory.testOngoing)
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

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("End test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 40.0f)))
    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        memory.testStarted = false;
        memory.testOngoing = false;
    }
    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::EndDisabled();
    }
    ImGui::PopStyleColor();
}

static void drawDiscoverMDButton(CommonMemory& memory)
{
    ImGui::Separator();

    if (discoverOngoing)
    {
        ImGui::BeginDisabled();
    }

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Discover MD", ImVec2(leftMenuBarWidth - (margin * 2.0f), 20.0f)))
    {
        memory.buttonDiscoverMdPressed = true;
    }

    if (discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

static void drawParametersVelocity(CommonMemory& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    const int numberOfColumns = 3;

    ImGui::Separator();

    ImGui::Spacing();
    CenterText("Velocity loop - PID tuning parameters");
    ImGui::Spacing();

    if (ImGui::BeginTable("ParamTableVelocity", numberOfColumns, flags))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kp_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kp_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Ki_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Ki_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kd_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.integralMax_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &integralMax_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset Velocity Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_velSlider          = 0.0f;
        Ki_velSlider          = 0.0f;
        Kd_velSlider          = 0.0f;
        integralMax_velSlider = 0.0f;
    }

    ImGui::Spacing();
}

static void drawParametersPosition(CommonMemory& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    const int numberOfColumns = 3;
    ImGui::Separator();

    ImGui::Spacing();
    CenterText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kp_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kp_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Ki_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Ki_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kd_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.integralMax_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &integralMax_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset Position Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_posSlider          = 0.0f;
        Ki_posSlider          = 0.0f;
        Kd_posSlider          = 0.0f;
        integralMax_posSlider = 0.0f;
    }
    ImGui::Spacing();
}

static void drawParametersImpedance(CommonMemory& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    const int numberOfColumns = 3;
    ImGui::Separator();

    ImGui::Spacing();
    CenterText("Impedance PD tuner");
    ImGui::Spacing();

    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags))
    {
        ImGui::TableSetupColumn("Variable name");
        ImGui::TableSetupColumn("Read value");
        ImGui::TableSetupColumn("Write value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kp impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kp_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kp_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputFloat("##hidden_label", &Kd_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset Impedance Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_impSlider = 0.0f;
        Kd_impSlider = 0.0f;
    }
    ImGui::Spacing();
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
    mab::canId_t              chosenID     = 0;
    std::string               chosenIDname = "";

    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        mdIDs    = memory.mdIDs;
        chosenID = memory.chosenID;
    }

    if (mdIDs.empty())
    {
        selectedMD   = false;
        selectedMode = false;
    }

    if (discoverOngoing && mdIDs.empty())
    {
        double time       = ImGui::GetTime();
        int    dotCounter = (int)(time * 2.5) % 4;

        chosenIDstr = "Discover ongoing" + std::string("...").substr(0, dotCounter);
        selectedMD  = false;
        ImGui::BeginDisabled();
    }
    else if (mdIDs.empty())
    {
        chosenIDstr = "No MDs available.";
        ImGui::BeginDisabled();
    }
    else if (!selectedMD)
        chosenIDstr = "Select Your MD";

    if (ImGui::BeginCombo("MD Select", chosenIDstr.c_str()))
    {
        if (ImGui::Selectable("None"))
        {
            selectedMD = false;
        }
        for (const auto& id : mdIDs)
        {
            chosenIDname = "MD" + std::to_string(int((id)));

            bool selectedID = (chosenID == id);

            if (ImGui::Selectable(chosenIDname.c_str()))
            {
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.chosenID     = id;
                memory.selectedMDid = true;
                selectedMD          = true;
                chosenIDstr         = chosenIDname;
                discoverOngoing     = false;
            }
            if (selectedID)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (discoverOngoing && mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
    else if (mdIDs.empty())
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

// static bool drawOrangeInputFloat(
//     const char* label, float* v, float step, float step_fast, const char* format)
// {
//     ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 12.0f));
//     ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)mabColor);
//     ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
//     ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
//     bool valueChanged = ImGui::InputFloat(label, v, step, step_fast, format);

//     ImGui::PopStyleColor(3);
//     ImGui::PopStyleVar();
//     return valueChanged;
// }

static void downloadParameters(CommonMemory& memory, mab::MD& md)
{
    mab::MD::Error_t err = md.readRegisters(md.m_mdRegisters.motorImpPidKp,
                                            md.m_mdRegisters.motorImpPidKd,
                                            md.m_mdRegisters.motorVelPidKp,
                                            md.m_mdRegisters.motorVelPidKi,
                                            md.m_mdRegisters.motorVelPidKd,
                                            md.m_mdRegisters.motorPosPidKp,
                                            md.m_mdRegisters.motorPosPidKi,
                                            md.m_mdRegisters.motorPosPidKd,
                                            md.m_mdRegisters.positionWindow,
                                            md.m_mdRegisters.velocityWindow);

    mab::MD::Error_t err2 = md.readRegisters(md.m_mdRegisters.maxVelocity,
                                             md.m_mdRegisters.positionLimitMax,
                                             md.m_mdRegisters.positionLimitMin,
                                             md.m_mdRegisters.maxTorque,
                                             md.m_mdRegisters.maxAcceleration,
                                             md.m_mdRegisters.maxDeceleration,
                                             md.m_mdRegisters.motorVelPidWindup,
                                             md.m_mdRegisters.motorPosPidWindup);

    if (err != mab::MD::Error_t::OK && err2 != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(memory.mtx);
            memory.Kp_vel          = float(md.m_mdRegisters.motorVelPidKp.value);
            memory.Ki_vel          = float(md.m_mdRegisters.motorVelPidKi.value);
            memory.Kd_vel          = float(md.m_mdRegisters.motorVelPidKd.value);
            memory.integralMax_vel = float(md.m_mdRegisters.motorVelPidWindup.value);

            memory.Kp_pos          = float(md.m_mdRegisters.motorPosPidKp.value);
            memory.Ki_pos          = float(md.m_mdRegisters.motorPosPidKi.value);
            memory.Kd_pos          = float(md.m_mdRegisters.motorPosPidKd.value);
            memory.integralMax_pos = float(md.m_mdRegisters.motorPosPidWindup.value);

            memory.Kp_imp = float(md.m_mdRegisters.motorImpPidKp.value);
            memory.Kd_imp = float(md.m_mdRegisters.motorImpPidKd.value);
        }

        positionWindowSlider = float(md.m_mdRegisters.positionWindow.value);
        velocityWindowSlider = float(md.m_mdRegisters.velocityWindow.value);

        maxVelocityClamp     = float(md.m_mdRegisters.maxVelocity.value);
        maxPositionClamp     = float(md.m_mdRegisters.positionLimitMax.value);
        minPositionClamp     = float(md.m_mdRegisters.positionLimitMin.value);
        maxTorqueClamp       = float(md.m_mdRegisters.maxTorque.value);
        maxAccelerationClamp = float(md.m_mdRegisters.maxAcceleration.value);
        maxDecelerationClamp = float(md.m_mdRegisters.maxDeceleration.value);

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

    mab::canId_t min      = 0;
    mab::canId_t max      = 100;
    mab::canId_t chosenID = 0;

    constexpr mab::canId_t MAX_VALID_ID = 0x7FF;

    while (isRunning)
    {
        bool          testStarted             = false;
        bool          testOngoing             = false;
        bool          updateParametersTest    = false;
        bool          buttonDiscoverMdPressed = false;
        bool          selectedMDid            = false;
        mab::MdMode_E currentMode             = mab::MdMode_E::IDLE;

        {
            std::lock_guard<std::mutex> lock(memory.mtx);
            testStarted             = memory.testStarted;
            testOngoing             = memory.testOngoing;
            updateParametersTest    = memory.updateParametersTest;
            buttonDiscoverMdPressed = memory.buttonDiscoverMdPressed;
            selectedMDid            = memory.selectedMDid;
            currentMode             = memory.currentMode;
            chosenID                = memory.chosenID;

            memory.updateParametersTest    = false;
            memory.buttonDiscoverMdPressed = false;
            memory.selectedMDid            = false;
        }

        for (auto& id : memory.mdIDs)
        {
            if (id == chosenID)
            {
                mab::MD md(id, candle);
                if (selectedMDid)
                {
                    md.init();
                    downloadParameters(memory, md);
                }

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

                if ((testStarted && currentMode != mab::MdMode_E::IDLE) || testOngoing)
                {
                    {
                        std::lock_guard<std::mutex> lock(memory.mtx);
                        memory.testOngoing = true;
                    }

                    testMD(memory, md);

                    md.readRegisters(md.m_mdRegisters.velocity,
                                     md.m_mdRegisters.position,
                                     md.m_mdRegisters.torque);

                    {
                        std::lock_guard<std::mutex> lock(memory.mtx);
                        memory.currentVelMeasured    = float(md.m_mdRegisters.velocity.value);
                        memory.currentPosMeasured    = float(md.m_mdRegisters.position.value);
                        memory.currentTorqueMeasured = float(md.m_mdRegisters.torque.value);
                    }
                }
            }
        }

        if (buttonDiscoverMdPressed)
        {
            discoverOngoing = true;
            {
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.mdIDs.clear();
            }
        }

        if (selectedMD)
        {
            min = 0;
            max = 100;
        }

        if (discoverOngoing)
        {
            for (const auto& id : mab::MD::discoverRangedMDs(candle, min, max))
            {
                memory.mdIDs.push_back(id);
            }
            min += 100;
            max += 100;
            if (max > MAX_VALID_ID)
            {
                discoverOngoing = false;
                min             = 0;
                max             = 100;
            }
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

    // int i   = 0;
    // int sum = 0;

    // Initialization ofo candlehardware thread
    std::thread hardware(candleLoop, std::ref(m_common), std::ref(isRunning));

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // auto start_time = std::chrono::high_resolution_clock::now();

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
        drawTestMenuBar(m_common, io);
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

        // i++;
        // auto end_time = std::chrono::high_resolution_clock::now();

        // auto duration =
        //     std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // sum += int(duration.count());

        // if (i == 10)
        // {
        //     std::cout << "Loop execution took: " << sum / 10 << " milliseconds.\n";
        //     i   = 0;
        //     sum = 0;
        // }
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
