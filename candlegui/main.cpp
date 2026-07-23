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
static void drawMenuTopBar(commonMemory_S& memory, ImGuiIO& io)
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

static void drawMenuLowerBar(commonMemory_S& memory, ImGuiIO& io)
{
    const ImGuiViewport* lowbarViewport = ImGui::GetMainViewport();

    ImVec2 lowbarPos =
        ImVec2(lowbarViewport->WorkPos.x,
               lowbarViewport->WorkPos.y + lowbarViewport->WorkSize.y - lowBarHeight);
    ImGui::SetNextWindowPos(lowbarPos, ImGuiCond_Always);

    ImVec2 mainSize = ImVec2(lowbarViewport->WorkSize.x, lowBarHeight);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Lower Bar", nullptr, flagsBackMenu))
    {
        ImGui::Text(
            "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    }
    ImGui::End();
}

static void drawMainMenu(commonMemory_S& memory, ImGuiIO& io, guiBuffers_S& buffers)
{
    const ImGuiViewport* mainMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(mainMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 mainPos =
        ImVec2(mainMenuViewport->WorkPos.x + leftMenuBarWidth, mainMenuViewport->WorkPos.y);
    ImGui::SetNextWindowPos(mainPos, ImGuiCond_Always);

    ImVec2 mainSize = ImVec2(mainMenuViewport->WorkSize.x - leftMenuBarWidth,
                             mainMenuViewport->WorkSize.y - lowBarHeight);
    ImGui::SetNextWindowSize(mainSize, ImGuiCond_Always);

    if (ImGui::Begin("Main menu", nullptr, flagsBackMenu))
    {
        if (systemON)
        {
            updatePlotData(memory, buffers, io);
            drawVelocityPlot(memory, buffers);
            drawPositionPlot(memory, buffers);
            drawTorquePlot(memory, buffers);
        }
    }

    ImGui::End();
}

static void drawLeftMenuBar(commonMemory_S& memory, ImGuiIO& io)
{
    const ImGuiViewport* leftMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(leftMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 windowSize =
        ImVec2(leftMenuBarWidth, leftMenuViewport->WorkSize.y - testMenuBarHeight - lowBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Main Menu", nullptr, flagsBackMenu))
    {
        bool testOngoing = memory.testOngoing;

        if (testOngoing)
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
        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::End();
}

static void drawTestMenuBar(commonMemory_S& memory, ImGuiIO& io)
{
    const ImGuiViewport* testMenuViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(testMenuViewport->WorkPos, ImGuiCond_Always);

    ImVec2 testPos = ImVec2(testMenuViewport->WorkPos.x,
                            testMenuViewport->WorkPos.y + testMenuViewport->WorkSize.y -
                                testMenuBarHeight - lowBarHeight);
    ImGui::SetNextWindowPos(testPos, ImGuiCond_Always);

    ImVec2 windowSize = ImVec2(leftMenuBarWidth, testMenuBarHeight);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    if (ImGui::Begin("Test menu", nullptr, flagsBackMenu))
    {
        drawTestEndButton(memory);
    }
    ImGui::End();
}

static void drawErrorMenuPopup(commonMemory_S& memory, ImGuiIO& io)
{
    const char* popupTitle = "Candle Error##ErrorPopup";

    ImGui::OpenPopup(popupTitle);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, mabColor);

    if (ImGui::BeginPopupModal(popupTitle, nullptr, flags))
    {
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("You forgot your Candle!");
        ImGui::Separator();

        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                           "Continue by connecting Candle via USB!");

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(1);
}

static void updatePlotData(commonMemory_S& memory, guiBuffers_S& buffers, ImGuiIO& io)
{
    static bool  lastTestStarted    = false;
    static float timeInTargetWindow = 0.0f;
    static float lastHardwareTime   = 0.0f;

    bool testStarted = memory.testStarted;

    if (testStarted && !lastTestStarted)
    {
        buffers.reset();
        buffers.readData   = memory.plotWriteData.load(std::memory_order_acquire);
        timeInTargetWindow = 0.0f;
        lastHardwareTime   = 0.0f;

        buffers.guiElapsedTime = 0.0f;
    }
    lastTestStarted = testStarted;

    if (testStarted)
    {
        buffers.guiElapsedTime += io.DeltaTime;
        uint32_t currentDataRead = memory.plotWriteData.load(std::memory_order_acquire);

        while (buffers.readData != currentDataRead)
        {
            plotPoints_S pP =
                memory.plotBuffer[buffers.readData % commonMemory_S::PLOT_BUFFER_SIZE];

            float dt_point   = pP.time - lastHardwareTime;
            lastHardwareTime = pP.time;

            if (pP.velocity > buffers.maxVel)
                buffers.maxVel = pP.velocity;
            if (pP.velocity < buffers.minVel)
                buffers.minVel = pP.velocity;
            if (pP.position > buffers.maxPos)
                buffers.maxPos = pP.position;
            if (pP.position < buffers.minPos)
                buffers.minPos = pP.position;
            if (pP.torque > buffers.maxTrq)
                buffers.maxTrq = pP.torque;
            if (pP.torque < buffers.minTrq)
                buffers.minTrq = pP.torque;

            bool inPosWindow = std::abs(pP.position - memory.targetPosition) <= positionWindow;
            bool inVelWindow = std::abs(pP.velocity - memory.targetVelocity) <= velocityWindow;

            switch (memory.currentMode)
            {
                case mab::MdMode_E::IDLE:
                    break;
                case mab::MdMode_E::VELOCITY_PID:
                    timeInTarget(inVelWindow, timeInTargetWindow, dt_point, memory);
                    break;
                case mab::MdMode_E::POSITION_PID:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point, memory);
                    break;
                case mab::MdMode_E::IMPEDANCE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point, memory);
                    break;
                case mab::MdMode_E::RAW_TORQUE:  // case unused
                    break;
                case mab::MdMode_E::VELOCITY_PROFILE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point, memory);
                    break;
                case mab::MdMode_E::POSITION_PROFILE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point, memory);
                    break;
                default:
                    break;
            }

            buffers.time[buffers.offset]      = pP.time;
            buffers.vel[buffers.offset]       = pP.velocity;
            buffers.pos[buffers.offset]       = pP.position;
            buffers.trq[buffers.offset]       = pP.torque;
            buffers.targetVel[buffers.offset] = memory.targetVelocity;
            buffers.targetPos[buffers.offset] = memory.targetPosition;
            buffers.targetTrq[buffers.offset] = memory.targetTorque;

            buffers.offset = (buffers.offset + 1) % guiBuffers_S::SIZE;
            buffers.readData++;
        }
    }
}

static void timeInTarget(bool&           inWindow,
                         float&          timeInTargetWindow,
                         float&          dt,
                         commonMemory_S& memory)
{
    if (inWindow)
    {
        timeInTargetWindow += dt;

        if (timeInTargetWindow >= targetHoldTime)
        {
            {
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.testStarted = false;
                memory.testOngoing = false;
            }
        }
    }
    else
    {
        timeInTargetWindow = 0.0f;
    }
}

static void drawVelocityPlot(commonMemory_S& memory, guiBuffers_S& buffers)
{
    ImPlotSpec spec;
    spec.Offset = buffers.offset;

    bool testStarted = memory.testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##VelocityPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 3)))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            buffers.time[(buffers.offset == 0) ? guiBuffers_S::SIZE - 1 : buffers.offset - 1];

        float oldestTime = buffers.time[buffers.offset];

        ImPlot::SetupAxisLimits(ImAxis_X1, oldestTime, currentTime, plotCondition);

        float bottomY =
            (buffers.minVel < 0.0f) ? (buffers.minVel - 0.2f * std::abs(buffers.minVel)) : 0.0f;
        float topY = (buffers.maxVel > 0.0f) ? (buffers.maxVel + 0.2f * buffers.maxVel) : 0.0f;

        if (memory.targetVelocity > topY)
            topY = memory.targetVelocity + 0.2f * memory.targetVelocity;
        if (memory.targetVelocity < bottomY)
            bottomY = memory.targetVelocity - 0.2f * std::abs(memory.targetVelocity);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Velocity(t)", buffers.time, buffers.vel, guiBuffers_S::SIZE, spec);

        if (memory.currentMode == mab::MdMode_E::VELOCITY_PID ||
            memory.currentMode == mab::MdMode_E::VELOCITY_PROFILE ||
            memory.currentMode == mab::MdMode_E::POSITION_PROFILE)
            ImPlot::PlotStairs(
                "Target Velocity", buffers.time, buffers.targetVel, guiBuffers_S::SIZE, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawPositionPlot(commonMemory_S& memory, guiBuffers_S& buffers)
{
    ImPlotSpec spec;
    spec.Offset      = buffers.offset;
    bool testStarted = memory.testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##PositionPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y / 2)))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            buffers.time[(buffers.offset == 0) ? guiBuffers_S::SIZE - 1 : buffers.offset - 1];

        float oldestTime = buffers.time[buffers.offset];

        ImPlot::SetupAxisLimits(ImAxis_X1, oldestTime, currentTime, plotCondition);

        float bottomY =
            (buffers.minPos < 0.0f) ? (buffers.minPos - 0.2f * std::abs(buffers.minPos)) : 0.0f;
        float topY = (buffers.maxPos > 0.0f) ? (buffers.maxPos + 0.2f * buffers.maxPos) : 0.0f;

        if (memory.targetPosition > topY)
            topY = memory.targetPosition + 0.2f * memory.targetPosition;
        if (memory.targetPosition < bottomY)
            bottomY = memory.targetPosition - 0.2f * std::abs(memory.targetPosition);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Position(t)", buffers.time, buffers.pos, guiBuffers_S::SIZE, spec);
        if (memory.currentMode == mab::MdMode_E::POSITION_PID ||
            memory.currentMode == mab::MdMode_E::POSITION_PROFILE ||
            memory.currentMode == mab::MdMode_E::IMPEDANCE ||
            memory.currentMode == mab::MdMode_E::VELOCITY_PROFILE)
            ImPlot::PlotStairs(
                "Target Position", buffers.time, buffers.targetPos, guiBuffers_S::SIZE, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawTorquePlot(commonMemory_S& memory, guiBuffers_S& buffers)
{
    ImPlotSpec spec;
    spec.Offset      = buffers.offset;
    bool testStarted = memory.testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    if (ImPlot::BeginPlot("##TorquegPlot", ImVec2(-1, ImGui::GetContentRegionAvail().y)))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            buffers.time[(buffers.offset == 0) ? guiBuffers_S::SIZE - 1 : buffers.offset - 1];

        float oldestTime = buffers.time[buffers.offset];

        ImPlot::SetupAxisLimits(ImAxis_X1, oldestTime, currentTime, plotCondition);

        float bottomY =
            (buffers.minTrq < 0.0f) ? (buffers.minTrq - 0.2f * std::abs(buffers.minTrq)) : 0.0f;
        float topY = (buffers.maxTrq > 0.0f) ? (buffers.maxTrq + 0.2f * buffers.maxTrq) : 0.0f;

        if (memory.targetTorque > topY)
            topY = memory.targetTorque + 0.2f * memory.targetTorque;
        if (memory.targetTorque < bottomY)
            bottomY = memory.targetTorque - 0.2f * std::abs(memory.targetTorque);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Torque(t)", buffers.time, buffers.trq, guiBuffers_S::SIZE, spec);
        if (memory.currentMode == mab::MdMode_E::IMPEDANCE)
            ImPlot::PlotStairs(
                "Target Torque", buffers.time, buffers.targetTrq, guiBuffers_S::SIZE, spec);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

static void drawSetTargetVelocity()
{
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Velocity");
    if (drawBigInputFloat(
            "##Target Velocity", &targetVelocitySlider, 1.0f, 2.0f, "%.2f", leftMenuBarWidth))
    {
        targetVelocitySlider =
            std::clamp(targetVelocitySlider, -maxVelocityClamp, maxVelocityClamp);
    }
}

static void drawSetTargetPosition()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Position");
    if (drawBigInputFloat(
            "##Target Position", &targetPositionSlider, 1.0f, 2.0f, "%.2f", leftMenuBarWidth))
    {
        targetPositionSlider = std::clamp(targetPositionSlider, minPositionClamp, maxPositionClamp);
    }
}

static void drawSetTargetTorque()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Target Torque");
    if (drawBigInputFloat(
            "##Target Torque", &targetTorqueSlider, 1.0f, 2.0f, "%.2f", leftMenuBarWidth))
    {
        targetTorqueSlider = std::clamp(targetTorqueSlider, -maxTorqueClamp, maxTorqueClamp);
    }
}

static void drawSetTargetAcceleration()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Acceleration");
    if (drawBigInputFloat(
            "##Acceleration", &targetAccelerationSlider, step, step_fast, "%.2f", leftMenuBarWidth))
    {
        targetAccelerationSlider = std::clamp(targetAccelerationSlider, 0.0f, maxAccelerationClamp);
    }
}

static void drawSetTargetDeceleration()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Deceleration");
    if (drawBigInputFloat(
            "##Deceleration", &targetDecelerationSlider, step, step_fast, "%.2f", leftMenuBarWidth))
    {
        targetDecelerationSlider = std::clamp(targetDecelerationSlider, 0.0f, maxDecelerationClamp);
    }
}

static void drawSetPositionWindow()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Position Window");
    drawBigInputFloat(
        "##Position Window", &positionWindowSlider, step, step_fast, "%.2f", leftMenuBarWidth);
}

static void drawSetVelocityWindow()
{
    ImGui::Spacing();
    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("Velocity Window");
    drawBigInputFloat(
        "##Velocity Window", &velocityWindowSlider, step, step_fast, "%.2f", leftMenuBarWidth);
}

static void drawDiscoverMDButton(commonMemory_S& memory)
{
    ImGui::Spacing();
    std::string chosenIDname = "";

    if (discoverOngoing)
    {
        ImGui::BeginDisabled();
        double  time       = ImGui::GetTime();
        uint8_t dotCounter = (uint8_t)(time * 2.5) % 4;

        chosenIDname = "Discover ongoing" + std::string("...").substr(0, dotCounter);
    }
    else
    {
        chosenIDname = "Discover MD";
    }

    buttonStyle();
    ImGui::SetCursorPosX(paddingButtons);
    if (ImGui::Button(chosenIDname.c_str(),
                      ImVec2(leftMenuBarWidth - (paddingButtons * 2.0f), mediumButtonHeight)))
    {
        memory.buttonDiscoverMdPressed = true;
    }
    endButtonStyle();

    if (discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

static void drawParametersVelocity(commonMemory_S& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Velocity loop - PID tuning parameters");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTableVelocity", numberOfColumns, flags, tableSize))
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
        buttonColorInputFloat("##hidden_label", &Kp_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Ki_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &Ki_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &Kd_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.integralMax_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &integralMax_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Velocity Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        Kp_velSlider          = 0.0f;
        Ki_velSlider          = 0.0f;
        Kd_velSlider          = 0.0f;
        integralMax_velSlider = 0.0f;
    }
    endButtonStyle();

    ImGui::Spacing();
}

static void drawParametersPosition(commonMemory_S& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Position loop - PID tuning parameters");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags, tableSize))
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
        buttonColorInputFloat("##hidden_label", &Kp_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Ki_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &Ki_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &Kd_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.integralMax_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &integralMax_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Position Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        Kp_posSlider          = 0.0f;
        Ki_posSlider          = 0.0f;
        Kd_posSlider          = 0.0f;
        integralMax_posSlider = 0.0f;
    }
    endButtonStyle();
    ImGui::Spacing();
}

static void drawParametersImpedance(commonMemory_S& memory)
{
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    ImGui::Spacing();
    centerText("Impedance PD tuner");
    ImGui::Spacing();

    ImGui::SetCursorPosX(paddingButtons);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * paddingButtons, 0.0f);
    if (ImGui::BeginTable("ParamTablePosition", numberOfColumns, flags, tableSize))
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
        buttonColorInputFloat("##hidden_label", &Kp_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", memory.Kd_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &Kd_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }
    ImGui::Spacing();
    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Impedance Parameters", ImVec2(leftMenuBarWidth / 2.0f, 30.0f)))
    {
        Kp_impSlider = 0.0f;
        Kd_impSlider = 0.0f;
    }
    endButtonStyle();
    ImGui::Spacing();
}

static void drawTestEndButton(commonMemory_S& memory)
{
    ImVec4 colorNormal, colorHovered, colorActive;
    float  borderSize;
    bool   testStarted = memory.testStarted;

    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::BeginDisabled();
    }

    if (testStarted)
    {
        borderSize   = 0.0f;
        colorNormal  = mabColor;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }
    else
    {
        borderSize   = 1.0f;
        colorNormal  = buttonColor;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, borderSize);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushStyleColor(ImGuiCol_Border, mabColor);
    ImGui::PushStyleColor(ImGuiCol_Button, colorNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);

    ImGui::SetCursorPosX(paddingButtons);
    if (ImGui::Button(memory.testStarted ? "End test" : "Test",
                      ImVec2(leftMenuBarWidth - (paddingButtons * 2.0f), 80.0f)))
    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        memory.testStarted = !memory.testStarted;
    }

    if (!selectedMode || memory.currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::EndDisabled();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

static void drawSelectModeButton(commonMemory_S& memory)
{
    ImGui::Spacing();

    if (!selectedMD)
    {
        std::lock_guard<std::mutex> lock(memory.mtx);
        memory.currentMode = mab::MdMode_E::IDLE;
        ImGui::BeginDisabled();
    }

    std::string comboText = "MD Motion Mode";
    comboStyle(comboText.c_str());
    if (ImGui::BeginCombo("##MD Motion Mode", getModeName(memory.currentMode)))
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
    endComboStyle();

    if (!selectedMD)
    {
        ImGui::EndDisabled();
    }
}

static void comboStyle(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, mabColorHovered);
    ImGui::PushStyleColor(ImGuiCol_Header, mabColorHovered);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);

    ImVec2 currentPadding = ImGui::GetStyle().FramePadding;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(currentPadding.x, roundingFrameButton));

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::Text("%s", text);

    ImGui::SetCursorPosX(paddingButtons);
    ImGui::SetNextItemWidth(leftMenuBarWidth - 2 * paddingButtons);
}

static void endComboStyle()
{
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(7);
}

static void buttonStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
}

static void endButtonStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

static void drawSelectMDButton(commonMemory_S& memory)
{
    ImGui::Spacing();

    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID     = 0;
    std::string               chosenIDname = "";

    mdIDs    = memory.mdIDs;
    chosenID = memory.chosenID;

    if (mdIDs.empty())
    {
        selectedMD   = false;
        selectedMode = false;
    }

    if (mdIDs.empty())
    {
        chosenIDstr = "No MDs available.";
        ImGui::BeginDisabled();
    }
    else if (!selectedMD)
        chosenIDstr = "Select Your MD";

    std::string comboText = "MD Select";
    comboStyle(comboText.c_str());
    if (ImGui::BeginCombo("##MD Select", chosenIDstr.c_str()))
    {
        if (ImGui::Selectable("None"))
        {
            selectedMD = false;
        }
        for (const mab::canId_t& id : mdIDs)
        {
            chosenIDname = "MD" + std::to_string(uint16_t((id)));

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
    endComboStyle();

    if (discoverOngoing && mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
    else if (mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
}

static void testMD(commonMemory_S& memory, mab::MD& md)
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
            md.setProfileDeceleration(memory.targetDeceleration);
            break;
        case mab::MdMode_E::POSITION_PROFILE:
            md.setTargetPosition(memory.targetPosition);
            md.setTargetVelocity(memory.targetVelocity);
            md.setProfileAcceleration(memory.targetAcceleration);
            md.setProfileDeceleration(memory.targetDeceleration);
            break;
        default:
            break;
    }
}

static void centerText(const char* text)
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
    const char* label, float* v, float step, float step_fast, const char* format, float windowWidth)
{
    bool valueChanged = false;

    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, mabColor);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, mabColor);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(roundingFrameButton, roundingFrameButton));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushID(label);
    float frameHeight = ImGui::GetFrameHeight();
    float buttonSize  = frameHeight;
    float spacing     = ImGui::GetStyle().ItemInnerSpacing.x;

    float interactableWidth = ImGui::CalcItemWidth();
    float inputWidth        = interactableWidth - (buttonSize + spacing) * 2.0f;

    float labelWidth = 0.0f;

    float totalWidgetWidth = interactableWidth + labelWidth;

    if (windowWidth > totalWidgetWidth)
    {
        float offsetX = (windowWidth - totalWidgetWidth) / 2.0f;
        ImGui::SetCursorPosX(offsetX);
    }

    ImGui::PushButtonRepeat(true);

    if (ImGui::Button("-", ImVec2(buttonSize, 0)))
    {
        *v -= ImGui::GetIO().KeyCtrl ? step_fast : step;
        valueChanged = true;
    }
    ImGui::SameLine(0, spacing);

    ImGui::SetNextItemWidth(inputWidth);
    if (ImGui::InputFloat("##input", v, 0.0f, 0.0f, format, ImGuiInputTextFlags_None))
    {
        valueChanged = true;
    }
    ImGui::SameLine(0, spacing);

    if (ImGui::Button("+", ImVec2(buttonSize, 0)))
    {
        *v += ImGui::GetIO().KeyCtrl ? step_fast : step;
        valueChanged = true;
    }

    ImGui::PopButtonRepeat();

    ImGui::PopID();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(6);

    ImGui::SetWindowFontScale(1.0f);

    return valueChanged;
}

static bool buttonColorInputFloat(const char* label,
                                  float*      v,
                                  float       step      = 0.0f,
                                  float       step_fast = 0.0f,
                                  const char* format    = "%.3f")
{
    ImVec4 bgColor = ImVec4(0.167f, 0.165f, 0.196f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bgColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bgColor);
    bool valueChanged = ImGui::InputFloat(label, v, step, step_fast, format);
    ImGui::PopStyleColor(3);
    return valueChanged;
}

static void downloadParameters(commonMemory_S& memory, mab::MD& md)
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
        std::cout << "Error reading registers: " << static_cast<uint8_t>(err) << "\n";
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

static void updateVelParameters(commonMemory_S& memory)
{
    memory.Kp_vel          = Kp_velSlider;
    memory.Ki_vel          = Ki_velSlider;
    memory.Kd_vel          = Kd_velSlider;
    memory.integralMax_vel = integralMax_velSlider;
}

static void updatePosParameters(commonMemory_S& memory)
{
    memory.Kp_pos          = Kp_posSlider;
    memory.Ki_pos          = Ki_posSlider;
    memory.Kd_pos          = Kd_posSlider;
    memory.integralMax_pos = integralMax_posSlider;
}

static void updateImpParameters(commonMemory_S& memory)
{
    memory.Kp_imp = Kp_impSlider;
    memory.Kd_imp = Kd_impSlider;
}

const char* errorToString(mab::candleTypes::Error_t error)
{
    switch (error)
    {
        case mab::candleTypes::Error_t::OK:
            return "OK";
        case mab::candleTypes::Error_t::DEVICE_NOT_CONNECTED:
            return "DEVICE_NOT_CONNECTED";
        case mab::candleTypes::Error_t::INITIALIZATION_ERROR:
            return "INITIALIZATION_ERROR";
        case mab::candleTypes::Error_t::UNINITIALIZED:
            return "UNINITIALIZED";
        case mab::candleTypes::Error_t::DATA_TOO_LONG:
            return "DATA_TOO_LONG";
        case mab::candleTypes::Error_t::DATA_EMPTY:
            return "DATA_EMPTY";
        case mab::candleTypes::Error_t::RESPONSE_TIMEOUT:
            return "RESPONSE_TIMEOUT";
        case mab::candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING:
            return "CAN_DEVICE_NOT_RESPONDING";
        case mab::candleTypes::Error_t::TRANSMITTER_ERROR:
            return "TRANSMITTER_ERROR";
        case mab::candleTypes::Error_t::RECEIVER_ERROR:
            return "RECEIVER_ERROR";
        case mab::candleTypes::Error_t::INVALID_ID:
            return "INVALID_ID";
        case mab::candleTypes::Error_t::BAD_RESPONSE:
            return "BAD_RESPONSE";
        case mab::candleTypes::Error_t::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        default:
            return "UNDEFINED";
    }
}

void candleLoop(commonMemory_S& memory, std::atomic<bool>& isRunning)
{
    mab::Candle* candle = nullptr;

    mab::canId_t min = 0;
    mab::canId_t max = 100;

    const std::chrono::microseconds                    dt = std::chrono::microseconds(200);
    std::chrono::time_point<std::chrono::steady_clock> nextExecTime =
        std::chrono::steady_clock::now();

    static std::chrono::time_point<std::chrono::steady_clock> testStartTime =
        std::chrono::steady_clock::now();
    static bool hardwareLastTestStarted = false;

    constexpr mab::canId_t MAX_VALID_ID = 0x7FF;

    int timeoutCounter = 0;

    while (isRunning)
    {
        bool testStarted             = memory.testStarted.load();
        bool updateParametersTest    = memory.updateParametersTest.exchange(false);
        bool buttonDiscoverMdPressed = memory.buttonDiscoverMdPressed.exchange(false);
        bool selectedMDid            = memory.selectedMDid.exchange(false);

        mab::MdMode_E currentMode;
        mab::canId_t  chosenID;

        currentMode = memory.currentMode;
        chosenID    = memory.chosenID;

        if (candle == nullptr)
        {
            auto busType =
                std::make_unique<mab::USB>(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID);

            if (busType->connect() == mab::I_CommunicationInterface::Error_t::OK)
            {
                candle =
                    mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M, std::move(busType));
                if (candle != nullptr)
                {
                    memory.candleAvailable = true;
                }
            }
        }

        if (candle != nullptr)
        {
            mab::candleTypes::Error_t errMsg = candle->legacyCheckConnection();

            if (errMsg == mab::candleTypes::Error_t::RESPONSE_TIMEOUT ||
                errMsg == mab::candleTypes::Error_t::RECEIVER_ERROR)
            {
                timeoutCounter += 1;
            }

            if ((errMsg != mab::candleTypes::Error_t::OK &&
                 errMsg != mab::candleTypes::Error_t::RESPONSE_TIMEOUT &&
                 errMsg != mab::candleTypes::Error_t::RECEIVER_ERROR) ||
                timeoutCounter > 5)
            {
                std::cout << "Error: " << errorToString(errMsg) << std::endl;
                std::lock_guard<std::mutex> lock(memory.mtx);
                memory.testStarted      = false;
                memory.testOngoing      = false;
                memory.candleAvailable  = false;
                buttonDiscoverMdPressed = false;
                discoverOngoing         = false;

                min = 0;
                max = 100;

                mab::detachCandle(candle);
                candle = nullptr;
            }
            else
                timeoutCounter = 0;

            mab::MD md(chosenID, candle);

            if (selectedMDid)
            {
                md.init();
                downloadParameters(memory, md);
            }

            if (!testStarted && hardwareLastTestStarted)
            {
                {
                    std::lock_guard<std::mutex> lock(memory.mtx);
                    memory.testOngoing = false;
                }
                md.disable();
            }

            if (testStarted && !hardwareLastTestStarted)
            {
                testStartTime               = std::chrono::steady_clock::now();
                memory.updateParametersTest = true;
            }
            hardwareLastTestStarted = testStarted;

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
                        memory.targetVelocity = targetVelocitySlider;
                        memory.targetPosition = 0.0f;
                        positionWindow        = 0.01;
                        velocityWindow        = velocityWindowSlider;
                        updateVelParameters(memory);
                        md.setVelocityPIDparam(
                            memory.Kp_vel, memory.Ki_vel, memory.Kd_vel, memory.integralMax_vel);
                        break;
                    case mab::MdMode_E::POSITION_PID:
                        memory.targetPosition = targetPositionSlider;
                        memory.targetVelocity = 0.0f;
                        velocityWindow        = 0.01;
                        positionWindow        = positionWindowSlider;
                        updatePosParameters(memory);
                        md.setPositionPIDparam(
                            memory.Kp_pos, memory.Ki_pos, memory.Kd_pos, memory.integralMax_pos);
                        break;
                    case mab::MdMode_E::IMPEDANCE:
                        memory.targetPosition = targetPositionSlider;
                        updateImpParameters(memory);
                        md.setImpedanceParams(memory.Kp_imp, memory.Kd_imp);
                        break;
                    case mab::MdMode_E::RAW_TORQUE:  // case unused
                        break;
                    case mab::MdMode_E::VELOCITY_PROFILE:
                        memory.targetPosition     = targetPositionSlider;
                        memory.targetVelocity     = targetVelocitySlider;
                        memory.targetAcceleration = targetAccelerationSlider;
                        memory.targetDeceleration = targetDecelerationSlider;
                        positionWindow            = 0.01;
                        velocityWindow            = velocityWindowSlider;
                        updateVelParameters(memory);
                        updatePosParameters(memory);
                        md.setVelocityPIDparam(
                            memory.Kp_vel, memory.Ki_vel, memory.Kd_vel, memory.integralMax_vel);
                        md.setPositionPIDparam(
                            memory.Kp_pos, memory.Ki_pos, memory.Kd_pos, memory.integralMax_pos);
                        break;
                    case mab::MdMode_E::POSITION_PROFILE:
                        memory.targetPosition     = targetPositionSlider;
                        memory.targetVelocity     = targetVelocitySlider;
                        memory.targetAcceleration = targetAccelerationSlider;
                        memory.targetDeceleration = targetDecelerationSlider;
                        velocityWindow            = 0.01;
                        positionWindow            = positionWindowSlider;
                        updateVelParameters(memory);
                        updatePosParameters(memory);
                        md.setVelocityPIDparam(
                            memory.Kp_vel, memory.Ki_vel, memory.Kd_vel, memory.integralMax_vel);
                        md.setPositionPIDparam(
                            memory.Kp_pos, memory.Ki_pos, memory.Kd_pos, memory.integralMax_pos);
                        break;
                    default:
                        break;
                }

                md.enable();
                memory.updateParametersTest = false;
            }

            if ((testStarted && currentMode != mab::MdMode_E::IDLE))
            {
                {
                    std::lock_guard<std::mutex> lock(memory.mtx);
                    memory.testOngoing = true;
                }

                std::chrono::time_point<std::chrono::steady_clock> now =
                    std::chrono::steady_clock::now();
                std::chrono::duration<float> elapsed         = now - testStartTime;
                float                        realTimeSeconds = elapsed.count();

                testMD(memory, md);

                md.readRegisters(
                    md.m_mdRegisters.velocity, md.m_mdRegisters.position, md.m_mdRegisters.torque);

                float vel = float(md.m_mdRegisters.velocity.value);
                float pos = float(md.m_mdRegisters.position.value);
                float trq = float(md.m_mdRegisters.torque.value);

                uint32_t writeData = memory.plotWriteData.load(std::memory_order_relaxed);

                memory.plotBuffer[writeData % commonMemory_S::PLOT_BUFFER_SIZE] = {
                    realTimeSeconds,
                    vel,
                    pos,
                    trq,
                    memory.targetVelocity,
                    memory.targetPosition,
                    memory.targetTorque};

                memory.plotWriteData.store(writeData + 1, std::memory_order_release);
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
                for (const mab::canId_t& id : mab::MD::discoverRangedMDs(candle, min, max))
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
        }
        nextExecTime += dt;

        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

        if (now < nextExecTime)
        {
            std::chrono::duration<float> timeLeft       = nextExecTime - now;
            std::chrono::milliseconds    sleepThreshold = std::chrono::milliseconds(1);

            if (timeLeft > sleepThreshold)
            {
                std::this_thread::sleep_for(timeLeft - sleepThreshold);
            }
            while (std::chrono::steady_clock::now() < nextExecTime)
            {
                std::this_thread::yield();
            }
        }
        else
            nextExecTime = std::chrono::steady_clock::now();
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
    io.Fonts->AddFontFromFileTTF("candlegui/fonts/font.ttf", 14.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Common memory init
    commonMemory_S m_common;
    guiBuffers_S   guiBuffers;

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

        if (m_common.candleAvailable)
        {
            drawMenuTopBar(m_common, io);
            drawMenuLowerBar(m_common, io);
            drawTestMenuBar(m_common, io);
            drawLeftMenuBar(m_common, io);
            drawMainMenu(m_common, io, guiBuffers);
        }
        else
        {
            drawErrorMenuPopup(m_common, io);

            ImGui::BeginDisabled();

            drawMenuTopBar(m_common, io);
            drawMenuLowerBar(m_common, io);
            drawTestMenuBar(m_common, io);
            drawLeftMenuBar(m_common, io);
            drawMainMenu(m_common, io, guiBuffers);

            ImGui::EndDisabled();
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
