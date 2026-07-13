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

static void drawMainMenu(ImGuiIO& io, mab::Candle* candle)
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
            drawVelocityPlot(io, candle);
            drawPositionPlot(io, candle);
            drawTorquePlot(io, candle);
        }
    }

    ImGui::End();
}

static void drawLeftMenuBar(ImGuiIO& io, mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

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
                drawToggleButton(candle);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                            1000.0f / io.Framerate,
                            io.Framerate);

                drawSelectModeButton();

                ImGui::Separator();
                drawTestButton(candle);
                drawEndTestButton(candle);

                switch (currentMode)
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
                    case mab::MdMode_E::RAW_TORQUE:
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
                drawDiscoverMDButton(candle);

                if (displayDetectedMD)
                {
                    CenterText("Detected MD List");
                    if (ImGui::BeginTable("Detected_md_table", 1, flagsTables))
                    {
                        for (const auto& id : mdV)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("MD%d", id.m_canId);
                        }
                        ImGui::EndTable();
                    }
                }
                drawEnableMDButton(candle);
                ImGui::SameLine();
                drawDisableMDButton(candle);

                if (ImGui::BeginPopup("my_select_popup"))
                {
                    ImGui::SeparatorText("Choose MD");
                    if (displayDetectedMD)
                    {
                        if (ImGui::Selectable("None"))
                        {
                            chosenID = 0;
                        }
                        for (const auto& id : mdV)
                        {
                            if (ImGui::Selectable("MD%d", id.m_canId))
                            {
                                // TODO add dynamic number selection to md detection
                                chosenID = id.m_canId;
                            }
                        }
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::Button("Select.."))
                    ImGui::OpenPopup("my_select_popup");

                ImGui::SameLine();
                if (ImGui::Button("Send Velocity"))
                {
                    if (displayDetectedMD)
                    {
                        targetVelocity = targetVelocitySlider;
                    }
                }

                if (displayDetectedMD)
                    ImGui::Text("You have chosen MD%d", chosenID);
                else
                    ImGui::Text("No MDs to choose from");

                ImGui::Checkbox("Demo Window", &show_demo_window);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        if (displayDetectedMD)
        {
            for (auto& id : mdV)
            {
                if (id.m_canId == chosenID)
                {
                    id.setTargetVelocity(targetVelocity);
                }
            }
        }

        if (testStarted)
        {
            testMD();
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
    if (drawOrangeInputFloat("Kp Vel", &Kp_vel, 0.01f, 0.1f))
    {
        Kp_vel = std::clamp(Kp_vel, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Ki Vel", &Ki_vel, 0.01f, 0.1f))
    {
        Ki_vel = std::clamp(Ki_vel, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Vel", &Kd_vel, 0.01f, 0.1f))
    {
        Kd_vel = std::clamp(Kd_vel, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Velocity Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_vel = 0.0f;
        Ki_vel = 0.0f;
        Kd_vel = 0.0f;
    }
}

static void drawPIDtunerPosition()
{
    ImGui::Separator();
    CenterText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    float sliderWidth = leftMenuBarWidth - 75.f;

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kp Pos", &Kp_pos, 0.01f, 0.1f))
    {
        Kp_pos = std::clamp(Kp_pos, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Ki Pos", &Ki_pos, 0.01f, 0.1f))
    {
        Ki_pos = std::clamp(Ki_pos, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Pos", &Kd_pos, 0.01f, 0.1f))
    {
        Kd_pos = std::clamp(Kd_pos, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Position Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_pos = 0.0f;
        Ki_pos = 0.0f;
        Kd_pos = 0.0f;
    }
}

static void drawPDtunerImpedance()
{
    ImGui::Separator();
    CenterText("Impedance PD tuner");
    ImGui::Spacing();

    float sliderWidth = leftMenuBarWidth - 75.f;

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kp Imp", &Kp_imp, 0.01f, 0.1f))
    {
        Kp_imp = std::clamp(Kp_imp, 0.0f, 100.f);
    }

    ImGui::SetNextItemWidth(sliderWidth);
    if (drawOrangeInputFloat("Kd Imp", &Kd_imp, 0.01f, 0.1f))
    {
        Kd_imp = std::clamp(Kd_imp, 0.0f, 100.f);
    }

    if (ImGui::Button("Reset Impedance Parameters", ImVec2(leftMenuBarWidth / 2.0f, 40.0f)))
    {
        Kp_imp = 0.0f;
        Kd_imp = 0.0f;
    }
}

static void drawVelocityPlot(ImGuiIO& io, mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

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

    if (testStarted && !lastTestStarted)
    {
        t                  = 0.0f;
        offset             = 0.0f;
        isTargetAchieved   = false;
        timeInTargetWindow = 0.0f;
        refresh_time       = ImGui::GetTime();

        minMeasured = (targetVelocity < 0.0f) ? targetVelocity : 0.0f;
        maxMeasured = (targetVelocity > 0.0f) ? targetVelocity : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = testStarted;

    float currentVel = mdV[0].getVelocity().first;

    if (testStarted && !isTargetAchieved)
    {
        if (currentVel > maxMeasured)
            maxMeasured = currentVel;
        if (currentVel < minMeasured)
            minMeasured = currentVel;

        bool inTargetWindow = std::abs(currentVel - targetVelocity) <= velocityWindow;

        if (inTargetWindow)
        {
            timeInTargetWindow += io.DeltaTime;

            if (timeInTargetWindow >= 0.25f)
            {
                isTargetAchieved = true;
                testStarted      = false;
                mdV[0].disable();
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
            targetValues[offset]      = targetVelocity;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##VelocityPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 3)))
    {
        ImPlotCond limitCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;

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

        if (currentMode == mab::MdMode_E::VELOCITY_PID ||
            currentMode == mab::MdMode_E::VELOCITY_PROFILE ||
            currentMode == mab::MdMode_E::POSITION_PROFILE)
            ImPlot::PlotStairs("Target Velocity", timeValues, targetValues, buffer_size, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawPositionPlot(ImGuiIO& io, mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

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

    if (testStarted && !lastTestStarted)
    {
        t                  = 0.0f;
        offset             = 0.0f;
        isTargetAchieved   = false;
        timeInTargetWindow = 0.0f;
        refresh_time       = ImGui::GetTime();

        minMeasured = (targetPosition < 0.0f) ? targetPosition : 0.0f;
        maxMeasured = (targetPosition > 0.0f) ? targetPosition : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = testStarted;

    float currentPos = mdV[0].getPosition().first;

    if (testStarted && !isTargetAchieved)
    {
        if (currentPos > maxMeasured)
            maxMeasured = currentPos;
        if (currentPos < minMeasured)
            minMeasured = currentPos;

        bool inTargetWindow = std::abs(currentPos - targetPosition) <= positionWindow;

        if (inTargetWindow)
        {
            timeInTargetWindow += io.DeltaTime;

            if (timeInTargetWindow >= 0.25f)
            {
                isTargetAchieved = true;
                testStarted      = false;
                mdV[0].disable();
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
            targetValues[offset]      = targetPosition;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##PositionPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 2)))
    {
        ImPlotCond limitCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;

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

        if (currentMode == mab::MdMode_E::POSITION_PID ||
            currentMode == mab::MdMode_E::POSITION_PROFILE ||
            currentMode == mab::MdMode_E::IMPEDANCE ||
            currentMode == mab::MdMode_E::VELOCITY_PROFILE)
            ImPlot::PlotStairs("Target Position", timeValues, targetValues, buffer_size, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawTorquePlot(ImGuiIO& io, mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

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

    if (testStarted && !lastTestStarted)
    {
        t            = 0.0f;
        offset       = 0.0f;
        refresh_time = ImGui::GetTime();

        minMeasured = (targetTorque < 0.0f) ? targetTorque : 0.0f;
        maxMeasured = (targetTorque > 0.0f) ? targetTorque : 0.0f;

        for (int i = 0; i < buffer_size; ++i)
        {
            timeValues[i]        = 0.0f;
            measurementValues[i] = 0.0f;
            targetValues[i]      = 0.0f;
        }
    }
    lastTestStarted = testStarted;

    float currentTorque = mdV[0].getTorque().first;

    if (testStarted)
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
            targetValues[offset]      = targetTorque;

            offset = (offset + 1) % buffer_size;
            refresh_time += 1.0f / 60.0f;
        }
    }

    ImPlotSpec spec;
    spec.Offset = offset;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##TorquePlot", ImVec2(-1, ImGui::GetContentRegionAvail().y)))
    {
        ImPlotCond limitCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;

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

        if (currentMode == mab::MdMode_E::RAW_TORQUE || currentMode == mab::MdMode_E::IMPEDANCE)
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

static void drawTestButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        testStarted = true;
        addMD100(candle);
    }
}

static void drawEndTestButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("End test", ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        testStarted = false;
        mdV[0].disable();
        mdV.clear();
    }
}

static void drawDiscoverMDButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Discover MD", ImVec2(leftMenuBarWidth - (margin * 2.0f), 80.0f)))
    {
        detectMD = !detectMD;

        mdV.clear();

        for (const auto& id : mab::MD::discoverMDs(candle))
        {
            mdV.emplace_back(id, candle);
            mdV.back().init();
        }

        if (!mdV.empty())
            displayDetectedMD = true;
        else
            displayDetectedMD = false;
    }
}

static void drawEnableMDButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }
    ImGui::SetCursorPosX(margin);
    if (ImGui::Button("Enable MD's", ImVec2(leftMenuBarWidth / 2 - (margin * 2.0f), 80.0f)))
    {
        for (auto& id : mdV)
        {
            if (id.setMotionMode(mab::MdMode_E::VELOCITY_PID) != mab::MD::Error_t::OK)
            {
                std::cout << "MD mode setting failed \n";
            }
            id.enable();
        }
    }
}

static void drawDisableMDButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }
    ImGui::SetCursorPosX(margin + leftMenuBarWidth / 2);
    if (ImGui::Button("Disable MD's", ImVec2(leftMenuBarWidth / 2 - (margin * 2.0f), 80.0f)))
    {
        targetVelocity = 0.0f;
        for (auto& id : mdV)
        {
            id.disable();
        }
    }
}

static void drawToggleButton(mab::Candle* candle)
{
    if (candle == nullptr)
    {
        return;
    }

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

static void drawSelectModeButton()
{
    ImGui::Separator();

    if (ImGui::BeginCombo("MD Motion Mode", getModeName(currentMode)))
    {
        if (ImGui::Selectable("None", currentMode == mab::MdMode_E::IDLE))
            currentMode = mab::MdMode_E::IDLE;
        if (ImGui::Selectable("Velocity PID", currentMode == mab::MdMode_E::VELOCITY_PID))
            currentMode = mab::MdMode_E::VELOCITY_PID;
        if (ImGui::Selectable("Position PID", currentMode == mab::MdMode_E::POSITION_PID))
            currentMode = mab::MdMode_E::POSITION_PID;
        if (ImGui::Selectable("Impedance PD", currentMode == mab::MdMode_E::IMPEDANCE))
            currentMode = mab::MdMode_E::IMPEDANCE;
        if (ImGui::Selectable("Raw Torque", currentMode == mab::MdMode_E::RAW_TORQUE))
            currentMode = mab::MdMode_E::RAW_TORQUE;
        if (ImGui::Selectable("Velocity Profile", currentMode == mab::MdMode_E::VELOCITY_PROFILE))
            currentMode = mab::MdMode_E::VELOCITY_PROFILE;
        if (ImGui::Selectable("Position Profile", currentMode == mab::MdMode_E::POSITION_PROFILE))
            currentMode = mab::MdMode_E::POSITION_PROFILE;

        ImGui::EndCombo();
    }
}

static void testMD()
{
    switch (currentMode)
    {
        case mab::MdMode_E::IDLE:

            break;
        case mab::MdMode_E::VELOCITY_PID:
            targetVelocity = targetVelocitySlider;
            targetPosition = 0.0f;
            positionWindow = 0.01;
            velocityWindow = velocityWindowSlider;
            mdV[0].setTargetVelocity(targetVelocity);
            break;
        case mab::MdMode_E::POSITION_PID:
            targetPosition = targetPositionSlider;
            targetVelocity = 0.0f;
            velocityWindow = 0.01;
            positionWindow = positionWindowSlider;
            mdV[0].setTargetPosition(targetPosition);
            break;
        case mab::MdMode_E::IMPEDANCE:
            targetPosition = targetPositionSlider;
            mdV[0].setTargetPosition(targetPosition);
            break;
        case mab::MdMode_E::RAW_TORQUE:
            targetTorque = targetTorqueSlider;
            mdV[0].setTargetTorque(targetTorque);
            break;
        case mab::MdMode_E::VELOCITY_PROFILE:
            targetPosition     = targetPositionSlider;
            targetVelocity     = targetVelocitySlider;
            targetAcceleration = targetAccelerationSlider;
            targetDecelration  = targetDecelrationSlider;
            positionWindow     = 0.01;
            velocityWindow     = velocityWindowSlider;
            mdV[0].setTargetPosition(targetPosition);
            mdV[0].setTargetVelocity(targetVelocity);
            mdV[0].setProfileAcceleration(targetAcceleration);
            mdV[0].setProfileDeceleration(targetDecelration);
            break;
        case mab::MdMode_E::POSITION_PROFILE:
            targetPosition     = targetPositionSlider;
            targetVelocity     = targetVelocitySlider;
            targetAcceleration = targetAccelerationSlider;
            targetDecelration  = targetDecelrationSlider;
            velocityWindow     = 0.01;
            positionWindow     = positionWindowSlider;
            mdV[0].setTargetPosition(targetPosition);
            mdV[0].setTargetVelocity(targetVelocity);
            mdV[0].setProfileAcceleration(targetAcceleration);
            mdV[0].setProfileDeceleration(targetDecelration);
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
        case mab::MdMode_E::RAW_TORQUE:
            return "Raw torque";
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

static void addMD100(mab::Candle* candle)
{
    mdV.clear();
    mdV.push_back(mab::MD(100, candle));

    if (mdV[0].init() != mab::MD::Error_t::OK)
    {
        std::cout << "MD not initialized\n";
    }

    mdV[0].zero();

    if (mdV[0].setMotionMode(currentMode) != mab::MD::Error_t::OK)
    {
        std::cout << "MD mode setting failed \n";
    }

    mdV[0].enable();
}

static void downloadParameters()
{
    mab::MD::Error_t err = mdV[0].readRegisters(registers.motorImpPidKp,
                                                registers.motorImpPidKd,
                                                registers.motorVelPidKp,
                                                registers.motorVelPidKi,
                                                registers.motorVelPidKd,
                                                registers.motorPosPidKp,
                                                registers.motorPosPidKi,
                                                registers.motorPosPidKd,
                                                registers.positionWindow,
                                                registers.velocityWindow);

    mab::MD::Error_t err2 = mdV[0].readRegisters(registers.maxVelocity,
                                                 registers.positionLimitMax,
                                                 registers.positionLimitMin,
                                                 registers.maxTorque,
                                                 registers.maxAcceleration,
                                                 registers.maxDeceleration);

    if (err != mab::MD::Error_t::OK && err2 != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
    }
    else
    {
        Kp_vel = float(registers.motorVelPidKp.value);
        Ki_vel = float(registers.motorVelPidKi.value);
        Kd_vel = float(registers.motorVelPidKd.value);

        Kp_pos = float(registers.motorPosPidKp.value);
        Ki_pos = float(registers.motorPosPidKi.value);
        Kd_pos = float(registers.motorPosPidKd.value);

        Kp_imp = float(registers.motorImpPidKp.value);
        Kd_imp = float(registers.motorImpPidKd.value);

        positionWindowSlider = float(registers.positionWindow.value);
        velocityWindowSlider = float(registers.velocityWindow.value);

        maxVelocityClamp     = float(registers.maxVelocity.value);
        maxPositionClamp     = float(registers.positionLimitMax.value);
        minPositionClamp     = float(registers.positionLimitMin.value);
        maxTorqueClamp       = float(registers.maxTorque.value);
        maxAccelerationClamp = float(registers.maxAcceleration.value);
        maxDecelerationClamp = float(registers.maxDeceleration.value);
    }
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
    auto candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                    mab::candleTypes::busTypes_t::USB);

    addMD100(candle);
    downloadParameters();

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
        drawLeftMenuBar(io, candle);
        drawMainMenu(io, candle);

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

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
    mab::detachCandle(candle);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
