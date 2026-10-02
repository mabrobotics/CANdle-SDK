#include "GUI.hpp"
#include "imgui.h"

GraphicInterface::GraphicInterface(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

void GraphicInterface::init()
{
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

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    std::string CANdleSDKVersion = CANDLESDK_VERSION;

    std::string appName = "MD GUI " + CANdleSDKVersion;

    m_window = glfwCreateWindow(1280, 960, appName.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
        return;

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);  // Enable vsync

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void GraphicInterface::close()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    if (m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void GraphicInterface::loop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(windowsPadding, windowsPadding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(windowsPadding, windowsPadding));

        if (ImGui::Begin("Main Window", nullptr, flagsBackMenu))
        {
            drawMenuTopBar();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, roundingFrameButton);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            float contentHeight = -(lowBarHeight + windowsPadding);
            if (ImGui::BeginChild("Content Menu", ImVec2(0, contentHeight), ImGuiChildFlags_None))
            {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                                    ImVec2(windowsPadding, windowsPadding));

                if (m_data->candleAvailable)
                {
                    updatePlotData();

                    switch (currentPage)
                    {
                        case TUNING:
                            drawTuningMenu();
                            break;
                        case MONITOR:
                            break;
                        case TEST_SCRIPT:
                            break;
                        case BUS_INFO:
                            break;
                        default:
                            drawTuningMenu();
                            break;
                    }
                }
                else
                {
                    drawErrorPopup();

                    ImGui::BeginDisabled();

                    switch (currentPage)
                    {
                        case TUNING:
                            drawTuningMenu();
                            break;
                        case MONITOR:
                            break;
                        case TEST_SCRIPT:
                            break;
                        case BUS_INFO:
                            break;
                        default:
                            drawTuningMenu();
                            break;
                    }

                    ImGui::EndDisabled();
                }
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);

            drawMenuBottomBar();
        }

        ImGui::End();
        ImGui::PopStyleVar(2);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w,
                     clear_color.y * clear_color.w,
                     clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

/*

Main menu draw functions

*/

void GraphicInterface::drawMenuTopBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(windowsPadding, insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(insideWindowPadding, insideWindowPadding));
    if (ImGui::BeginChild("TopBar", ImVec2(0, topBarHeight), ImGuiChildFlags_None))
    {
        buttonSelectStyle(currentPage == TUNING);
        drawTuningButton();
        endButtonSelectStyle();
        ImGui::SameLine();

        buttonSelectStyle(currentPage == MONITOR);
        drawMonitorButton();
        endButtonSelectStyle();
        ImGui::SameLine();

        buttonSelectStyle(currentPage == TEST_SCRIPT);
        drawTestScriptButton();
        endButtonSelectStyle();

        ImGui::SameLine();
        buttonSelectStyle(currentPage == BUS_INFO);
        drawBusInfoButton();
        endButtonSelectStyle();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawMenuBottomBar()
{
    if (ImGui::BeginChild("BottomBar", ImVec2(0, lowBarHeight), ImGuiChildFlags_None))
    {
        char text[128];
        snprintf(text,
                 sizeof(text),
                 "%.3f ms/frame (%.1f FPS)",
                 1000.0f / m_io.Framerate,
                 m_io.Framerate);

        float textWidth = ImGui::CalcTextSize(text).x;
        float spacing   = ImGui::GetStyle().ItemSpacing.x;

        if (ImGui::BeginChild(
                "BottomLeft", ImVec2(-(textWidth + spacing), 0), ImGuiChildFlags_None))
        {
            if (currentPage == TUNING)
                drawErrorMenuBar();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("BottomRight", ImVec2(0, 0), ImGuiChildFlags_None))
        {
            ImGui::TextUnformatted(text);
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void GraphicInterface::drawTestMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(leftMenuPadding, insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(insideWindowPadding / 2, insideWindowPadding));
    if (ImGui::BeginChild("Test menu", ImVec2(0, 0), ImGuiChildFlags_Borders, flagsBackMenu))
    {
        bool errorOccured = m_data->errorOccured;

        drawCursorMenu();

        ImGui::Separator();

        if (errorOccured)
        {
            ImGui::BeginDisabled();
        }

        drawTestManualButton();
        ImGui::SameLine();
        drawHelper(
            "Test MANUAL - hold button for manually controlled test.\nTest AUTO - press button for "
            "automatically provided test. Test ends when set value is in target window.");

        drawTestEndButton();

        if (errorOccured)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawErrorMenuBar()
{
    bool errorConnectionOccured    = m_data->errorConnectionOccured;
    bool errorQuickStatusOccured   = m_data->errorQuickStatusOccured;
    bool errorEncoderOccured       = m_data->errorEncoderOccured;
    bool errorHardwareOccured      = m_data->errorHardwareOccured;
    bool errorBridgeOccured        = m_data->errorBridgeOccured;
    bool errorMotionOccured        = m_data->errorMotionOccured;
    bool errorCommunicationOccured = m_data->errorCommunicationOccured;
    bool selectedMD                = m_data->selectedMD;

    ImGui::AlignTextToFramePadding();

    ImGui::Text("Connection status:");
    ImGui::SameLine();
    if (errorConnectionOccured)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, redColor);
        ImGui::Text("%s", m_data->errorMessage.c_str());
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, greenColor);
        ImGui::Text("%s", m_data->errorMessage.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::Text("Status:");
    ImGui::SameLine();
    if (errorQuickStatusOccured)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, redColor);
        ImGui::Text("%s", m_data->errorQuickStatusMessage.c_str());
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, greenColor);
        ImGui::Text("%s", m_data->errorQuickStatusMessage.c_str());
        ImGui::PopStyleColor();
    }

    if (selectedMD)
    {
        if (errorEncoderOccured)
        {
            ImGui::SameLine();
            ImGui::Text("Encoder status:");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, redColor);
            ImGui::Text("%s", m_data->errorEncoderStatusMessage.c_str());
            ImGui::PopStyleColor();
        }

        if (errorHardwareOccured)
        {
            ImGui::SameLine();
            ImGui::Text("Hardware status:");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, redColor);
            ImGui::Text("%s", m_data->errorHardwareStatusMessage.c_str());
            ImGui::PopStyleColor();
        }

        if (errorBridgeOccured)
        {
            ImGui::SameLine();
            ImGui::Text("Bridge status:");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, redColor);
            ImGui::Text("%s", m_data->errorBridgeStatusMessage.c_str());
            ImGui::PopStyleColor();
        }

        if (errorMotionOccured)
        {
            ImGui::SameLine();
            ImGui::Text("Motion status:");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, redColor);
            ImGui::Text("%s", m_data->errorMotionStatusMessage.c_str());
            ImGui::PopStyleColor();
        }

        if (errorCommunicationOccured)
        {
            ImGui::SameLine();
            ImGui::Text("Communication status:");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, redColor);
            ImGui::Text("%s", m_data->errorCommunicationStatusMessage.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        drawClearErrorsButton();

        ImGui::SameLine();
        drawRestorePlotsButton();
    }
}

void GraphicInterface::drawLeftMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(leftMenuPadding, windowsPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(insideWindowPadding / 2, insideWindowPadding));
    if (ImGui::BeginChild("Left Menu", ImVec2(0, -testMenuBarHeight), ImGuiChildFlags_Borders))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
        {
            ImGui::BeginDisabled();
        }
        drawDiscoverMDButton();
        drawSelectMDButton();
        drawSelectModeButton();

        switch (m_data->currentMode)
        {
            case mab::MdMode_E::IDLE:
                break;
            case mab::MdMode_E::VELOCITY_PID:
                drawParametersVelocity();
                drawSetTargetVelocity();
                drawSetVelocityWindow();
                drawSaveButton();
                break;
            case mab::MdMode_E::POSITION_PID:
                drawParametersVelocity();
                drawParametersPosition();
                drawSetTargetPosition();
                drawSetPositionWindow();
                drawSaveButton();
                break;
            case mab::MdMode_E::IMPEDANCE:
                drawParametersImpedance();
                drawSetTargetPosition();
                drawSetPositionWindow();
                drawSaveButton();
                break;
            case mab::MdMode_E::RAW_TORQUE:  // case unused
                drawSetTargetTorque();
                break;
            case mab::MdMode_E::VELOCITY_PROFILE:
                drawParametersVelocity();
                drawSetTargetVelocity();
                drawSetTargetAcceleration();
                drawSetTargetDeceleration();
                drawSetVelocityWindow();
                drawSaveButton();
                break;
            case mab::MdMode_E::POSITION_PROFILE:
                drawParametersVelocity();
                drawParametersPosition();
                drawSetTargetPosition();
                drawSetTargetAcceleration();
                drawSetTargetDeceleration();
                drawSetPositionWindow();
                drawSaveButton();
                break;
            default:
                break;
        }
        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawRightMenuBar()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(windowsPadding, insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(insideWindowPadding, insideWindowPadding));
    if (ImGui::BeginChild(
            "Right Menu", ImVec2(rightMenuBarWidth, 0), ImGuiChildFlags_Borders, flagsBackMenu))
    {
        mab::MdMode_E currentMode = m_data->currentMode;
        bool          testStarted = m_data->testStarted;

        if (testStarted || currentMode == mab::MdMode_E::IDLE)
        {
            ImGui::BeginDisabled();
        }

        ImGui::Text("Cursor Menu");

        ImGui::Separator();

        drawCheckboxCrosshairsButton();

        ImGui::Separator();

        switch (currentMode)
        {
            case mab::MdMode_E::VELOCITY_PID:
            case mab::MdMode_E::VELOCITY_PROFILE:

                ImGui::Text("Velocity plot");
                drawCheckboxVerCursorsVelButton();
                drawCheckboxHorCursorsVelButton();

                drawValuesVelocity();

                ImGui::Separator();

                ImGui::Text("Position plot");
                drawCheckboxVerCursorsPosButton();
                drawCheckboxHorCursorsPosButton();

                drawValuesPosition();

                ImGui::Separator();
                break;
            case mab::MdMode_E::POSITION_PID:
            case mab::MdMode_E::POSITION_PROFILE:
            case mab::MdMode_E::IMPEDANCE:

                ImGui::Text("Position plot");
                drawCheckboxVerCursorsPosButton();
                drawCheckboxHorCursorsPosButton();

                drawValuesPosition();

                ImGui::Separator();

                ImGui::Text("Velocity plot");
                drawCheckboxVerCursorsVelButton();
                drawCheckboxHorCursorsVelButton();

                drawValuesVelocity();

                ImGui::Separator();
                break;
            default:
                break;
        }

        ImGui::Text("Torque plot");
        drawCheckboxVerCursorsTrqButton();
        drawCheckboxHorCursorsTrqButton();

        drawValuesTorque();

        if (testStarted || currentMode == mab::MdMode_E::IDLE)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void GraphicInterface::drawMainMenu()
{
    float mainWidth = showCursorMenu ? -(rightMenuBarWidth + windowsPadding) : 0.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(windowsPadding, insideWindowPadding));
    if (ImGui::BeginChild(
            "Main menu", ImVec2(mainWidth, 0), ImGuiChildFlags_Borders, flagsBackMenu))
    {
        mab::MdMode_E currentModeLocal = m_data->currentMode;

        ImVec2 availableSpace        = ImGui::GetContentRegionAvail();
        float  heightWithoutSplitter = availableSpace.y - resizeButton;
        float  topHeight             = heightWithoutSplitter * menuTopHeightRatio;
        float  remainingHeight       = heightWithoutSplitter - topHeight;
        float  widthWithoutSplitter  = availableSpace.x - resizeButton;
        float  leftWidth             = widthWithoutSplitter * menuBottomWidthRatio;
        float  rightWidth            = widthWithoutSplitter - leftWidth;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

        ImPlotFlags plotFlags;
        if (buttonShowCrosshairsChecked)
        {
            plotFlags = ImPlotFlags_Crosshairs;
        }
        else
            plotFlags = ImPlotFlags_None;

        if (currentModeLocal == mab::MdMode_E::VELOCITY_PID ||
            currentModeLocal == mab::MdMode_E::VELOCITY_PROFILE)
        {
            if (buttonRestorePlotsPressed)
            {
                menuTopHeightRatio   = 0.5f;
                menuBottomWidthRatio = 0.5f;

                buttonRestorePlotsPressed = false;
            }

            drawVelocityPlot(ImVec2(-1, topHeight), plotFlags);

            ImGui::InvisibleButton("h_splitter", ImVec2(-1, resizeButton));
            if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

            if (ImGui::IsItemActive())
            {
                menuTopHeightRatio += ImGui::GetIO().MouseDelta.y / heightWithoutSplitter;
                menuTopHeightRatio = std::clamp(menuTopHeightRatio, 0.2f, 0.8f);
            }

            drawPositionPlot(ImVec2(leftWidth, remainingHeight), plotFlags);
            ImGui::SameLine(0.0f, 0.0f);

            ImGui::InvisibleButton("v_splitter", ImVec2(resizeButton, remainingHeight));
            if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            if (ImGui::IsItemActive())
            {
                menuBottomWidthRatio += ImGui::GetIO().MouseDelta.x / widthWithoutSplitter;
                menuBottomWidthRatio = std::clamp(menuBottomWidthRatio, 0.2f, 0.8f);
            }
            ImGui::SameLine(0.0f, 0.0f);

            drawTorquePlot(ImVec2(rightWidth, remainingHeight), plotFlags);
        }
        else if (currentModeLocal == mab::MdMode_E::POSITION_PID ||
                 currentModeLocal == mab::MdMode_E::POSITION_PROFILE ||
                 currentModeLocal == mab::MdMode_E::IMPEDANCE)
        {
            if (buttonRestorePlotsPressed)
            {
                menuTopHeightRatio   = 0.5f;
                menuBottomWidthRatio = 0.5f;

                buttonRestorePlotsPressed = false;
            }

            drawPositionPlot(ImVec2(-1, topHeight), plotFlags);

            ImGui::InvisibleButton("h_splitter", ImVec2(-1, resizeButton));
            if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

            if (ImGui::IsItemActive())
            {
                menuTopHeightRatio += ImGui::GetIO().MouseDelta.y / heightWithoutSplitter;
                menuTopHeightRatio = std::clamp(menuTopHeightRatio, 0.2f, 0.8f);
            }

            drawVelocityPlot(ImVec2(leftWidth, remainingHeight), plotFlags);
            ImGui::SameLine(0.0f, 0.0f);

            ImGui::InvisibleButton("v_splitter", ImVec2(resizeButton, remainingHeight));
            if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

            if (ImGui::IsItemActive())
            {
                menuBottomWidthRatio += ImGui::GetIO().MouseDelta.x / widthWithoutSplitter;
                menuBottomWidthRatio = std::clamp(menuBottomWidthRatio, 0.2f, 0.8f);
            }
            ImGui::SameLine(0.0f, 0.0f);

            drawTorquePlot(ImVec2(rightWidth, remainingHeight), plotFlags);
        }
        else
        {
            drawVelocityPlot(ImVec2(-1, availableSpace.y / 3), plotFlags);
            drawPositionPlot(ImVec2(-1, availableSpace.y / 3), plotFlags);
            drawTorquePlot(ImVec2(-1, availableSpace.y / 3), plotFlags);
        }

        ImGui::PopStyleVar();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void GraphicInterface::drawErrorPopup()
{
    const char* popupTitle = "CANdle Error##ErrorPopup";

    ImGui::OpenPopup(popupTitle);
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, mabColor);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(insideWindowPadding, insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(insideWindowPadding, insideWindowPadding));

    if (ImGui::BeginPopupModal(popupTitle, nullptr, flags))
    {
        ImGui::SetWindowFontScale(1.3f);

        if (!m_data->updatedVersion)
            ImGui::Text("Update CANdle drivers.");
        else
            ImGui::Text("You didn't light your CANdle!");

        ImGui::Separator();

        if (!m_data->updatedVersion)
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                               "Install new USB drivers by runing candlesdk-win-driver.exe!");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
                               "Continue by connecting CANdle via USB!");

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

void GraphicInterface::drawTuningMenu()
{
    if (ImGui::BeginChild("LeftColumn", ImVec2(leftMenuBarWidth, 0), ImGuiChildFlags_None))
    {
        drawLeftMenuBar();
        drawTestMenuBar();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    drawMainMenu();

    if (showCursorMenu)
    {
        ImGui::SameLine();
        drawRightMenuBar();
    }
}

/*

BUTTONS

*/

void GraphicInterface::drawTuningButton()
{
    ImVec2 tableSize = ImVec2(0, largeButtonHeight);

    if (ImGui::Button("Tuning", tableSize))
    {
        currentPage = TUNING;
    }
}

void GraphicInterface::drawMonitorButton()
{
    ImVec2 tableSize = ImVec2(0, largeButtonHeight);

    if (ImGui::Button("Monitor", tableSize))
    {
        currentPage = MONITOR;
    }
}

void GraphicInterface::drawTestScriptButton()
{
    ImVec2 tableSize = ImVec2(0, largeButtonHeight);

    if (ImGui::Button("Test Script", tableSize))
    {
        currentPage = TEST_SCRIPT;
    }
}

void GraphicInterface::drawBusInfoButton()
{
    ImVec2 tableSize = ImVec2(0, largeButtonHeight);

    if (ImGui::Button("Bus & Actuators", tableSize))
    {
        currentPage = BUS_INFO;
    }
}

void GraphicInterface::drawTestManualButton()
{
    bool          selectedMode               = m_data->selectedMode;
    mab::MdMode_E currentMode                = m_data->currentMode;
    bool          buttonAutomaticTestPressed = m_data->buttonAutomaticTestPressed;
    bool          buttonManualTestPressed    = m_data->buttonManualTestPressed;

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed)
    {
        ImGui::BeginDisabled();
    }

    buttonImportantStyle(buttonManualTestPressed);

    ImVec2 tableSize = ImVec2(leftMenuBarWidth - (leftMenuPadding * 2.0f), largeButtonHeight);

    ImGui::PushButtonRepeat(true);

    if (ImGui::Button("Apply Parameters & Test MANUAL", tableSize))
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->buttonManualTestPressed = true;
    }

    bool is_held = ImGui::IsItemActive();
    if (!buttonAutomaticTestPressed)
    {
        if (is_held)
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->testStarted                = true;
            m_data->buttonAutomaticTestPressed = false;
        }
        else
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->testStarted             = false;
            m_data->buttonManualTestPressed = false;
        }
    }

    ImGui::PopButtonRepeat();

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed)
    {
        ImGui::EndDisabled();
    }

    endButtonImportantStyle();
}

void GraphicInterface::drawTestEndButton()
{
    bool          testStarted             = m_data->testStarted;
    bool          selectedMode            = m_data->selectedMode;
    bool          buttonManualTestPressed = m_data->buttonManualTestPressed;
    bool          startCondition          = testStarted && !buttonManualTestPressed;
    mab::MdMode_E currentMode             = m_data->currentMode;

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonManualTestPressed)
    {
        ImGui::BeginDisabled();
    }

    buttonImportantStyle(startCondition);

    ImVec2 tableSize = ImVec2(leftMenuBarWidth - (leftMenuPadding * 2.0f), largeButtonHeight);

    if (ImGui::Button(
            m_data->buttonAutomaticTestPressed ? "End test" : "Apply Parameters & Test AUTO",
            tableSize))
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->testStarted                = !m_data->testStarted;
        m_data->buttonManualTestPressed    = false;
        m_data->buttonAutomaticTestPressed = true;
    }

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonManualTestPressed)
    {
        ImGui::EndDisabled();
    }

    endButtonImportantStyle();
}

void GraphicInterface::drawDiscoverMDButton()
{
    std::string chosenIDname = "";

    if (m_data->discoverOngoing)
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
    ImGui::SetCursorPosX(leftMenuPadding);
    if (ImGui::Button(chosenIDname.c_str(),
                      ImVec2(leftMenuBarWidth - (leftMenuPadding * 2.0f), largeButtonHeight)))
    {
        m_data->buttonDiscoverMdPressed = true;
    }
    endButtonStyle();

    if (m_data->discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

void GraphicInterface::drawSelectMDButton()
{
    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID     = 0;
    std::string               chosenIDname = "";

    bool selectedMD = m_data->selectedMD;

    mdIDs    = m_data->mdIDs;
    chosenID = m_data->chosenID;

    if (mdIDs.empty())
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->selectedMD   = false;
        m_data->selectedMode = false;
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
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->selectedMD = false;
        }
        for (const mab::canId_t& id : mdIDs)
        {
            chosenIDname = "MD" + std::to_string(uint16_t((id)));

            bool selectedID = (chosenID == id);

            if (ImGui::Selectable(chosenIDname.c_str()))
            {
                std::lock_guard<std::mutex> lock(m_data->mtx);
                m_data->chosenID              = id;
                m_data->buttonSelectMdPressed = true;
                m_data->selectedMD            = true;
                chosenIDstr                   = chosenIDname;
                m_data->discoverOngoing       = false;
            }
            if (selectedID)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    endComboStyle();

    if (m_data->discoverOngoing && mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
    else if (mdIDs.empty())
    {
        ImGui::EndDisabled();
    }
}

void GraphicInterface::drawSelectModeButton()
{
    bool selectedMD = m_data->selectedMD;

    if (!selectedMD)
    {
        std::lock_guard<std::mutex> lock(m_data->mtx);
        m_data->currentMode = mab::MdMode_E::IDLE;
        ImGui::BeginDisabled();
    }

    std::string comboText = "MD Motion Mode";
    comboStyle(comboText.c_str());
    if (ImGui::BeginCombo("##MD Motion Mode", getModeName(m_data->currentMode)))
    {
        if (ImGui::Selectable("None", m_data->currentMode == mab::MdMode_E::IDLE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::IDLE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Velocity PID", m_data->currentMode == mab::MdMode_E::VELOCITY_PID))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::VELOCITY_PID;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Position PID", m_data->currentMode == mab::MdMode_E::POSITION_PID))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::POSITION_PID;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Impedance PD", m_data->currentMode == mab::MdMode_E::IMPEDANCE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::IMPEDANCE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Velocity Profile",
                              m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::VELOCITY_PROFILE;
            m_data->selectedMode = true;
        }
        if (ImGui::Selectable("Position Profile",
                              m_data->currentMode == mab::MdMode_E::POSITION_PROFILE))
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->currentMode  = mab::MdMode_E::POSITION_PROFILE;
            m_data->selectedMode = true;
        }

        ImGui::EndCombo();
    }
    endComboStyle();

    if (!selectedMD)
    {
        ImGui::EndDisabled();
    }
}

void GraphicInterface::drawRestorePlotsButton()
{
    buttonStyle();
    if (ImGui::Button("Restore Plots", ImVec2(0, smallButtonHeight)))
    {
        buttonRestorePlotsPressed = true;
    }
    endButtonStyle();
}

void GraphicInterface::drawClearErrorsButton()
{
    buttonStyle();
    if (ImGui::Button("Clear Errors", ImVec2(0, smallButtonHeight)))
    {
        m_data->buttonClearErrorsPressed = true;
    }
    endButtonStyle();
}

void GraphicInterface::drawSaveButton()
{
    bool          selectedMode               = m_data->selectedMode;
    mab::MdMode_E currentMode                = m_data->currentMode;
    bool          buttonAutomaticTestPressed = m_data->buttonAutomaticTestPressed;
    bool          buttonManualTestPressed    = m_data->buttonManualTestPressed;
    bool          buttonSavePressed          = m_data->buttonSavePressed;

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed ||
        buttonManualTestPressed)
    {
        ImGui::BeginDisabled();
    }

    ImGui::Separator();

    ImGui::SetCursorPosX((leftMenuBarWidth - saveButtonWidth) / 2.0f);

    float saveButtonY = ImGui::GetWindowHeight() - (mediumButtonHeight + windowsPadding);

    float currentY = ImGui::GetCursorPosY();

    ImGui::SetCursorPosY(currentY > saveButtonY ? currentY : saveButtonY);

    buttonImportantStyle(buttonSavePressed);

    if (ImGui::Button("Save Config To Flash Memory", ImVec2(saveButtonWidth, mediumButtonHeight)))
    {
        ImGui::OpenPopup("Warning: Save config");
    }

    endButtonImportantStyle();

    ImGui::SameLine();
    drawHelper(
        "Choose this option only if you want to save configuration to drive's permament "
        "memory.\nSetting not neceserry for testing.");

    if (!selectedMode || currentMode == mab::MdMode_E::IDLE || buttonAutomaticTestPressed ||
        buttonManualTestPressed)
    {
        ImGui::EndDisabled();
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, mabColor);
    if (ImGui::BeginPopupModal("Warning: Save config", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SetWindowFontScale(1.3f);

        ImVec2 windowSize = ImGui::GetWindowSize();

        ImGui::Text("This action will overwrite your current config in drive's permament memory.");
        ImGui::Text("Are you sure you want to continue?");
        ImGui::Separator();

        ImGui::SetCursorPosX((windowSize.x - 248.f) / 2);
        buttonStyle();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            m_data->buttonSavePressed = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        endButtonStyle();

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(1);
}

void GraphicInterface::drawCheckboxCrosshairsButton()
{
    checkboxStyle();
    ImGui::Checkbox("Show Crosshairs", &buttonShowCrosshairsChecked);
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxVerCursorsVelButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Cursors Vel", &cursorsVerticalVelocityEnabled))
    {
        cursorAPositionVel = 0.2 * timeElapsedOnPlot;
        cursorBPositionVel = 0.8 * timeElapsedOnPlot;
    }
    ImGui::SameLine();
    ImGui::Text("Vertical Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxVerCursorsPosButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Cursors Pos", &cursorsVerticalPositionEnabled))
    {
        cursorAPositionPos = 0.2 * timeElapsedOnPlot;
        cursorBPositionPos = 0.8 * timeElapsedOnPlot;
    }
    ImGui::SameLine();
    ImGui::Text("Vertical Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxVerCursorsTrqButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Cursors Trq", &cursorsVerticalTorqueEnabled))
    {
        cursorAPositionTrq = 0.2 * timeElapsedOnPlot;
        cursorBPositionTrq = 0.8 * timeElapsedOnPlot;
    }
    ImGui::SameLine();
    ImGui::Text("Vertical Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxHorCursorsVelButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Horizontal Cursors Vel", &cursorsHorizontalVelocityEnabled))
    {
        cursorAHorPositionVel = minVel;
        cursorBHorPositionVel = maxVel;
    }
    ImGui::SameLine();
    ImGui::Text("Horizontal Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxHorCursorsPosButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Horizontal Cursors Pos", &cursorsHorizontalPositionEnabled))
    {
        cursorAHorPositionPos = minPos;
        cursorBHorPositionPos = maxPos;
    }
    ImGui::SameLine();
    ImGui::Text("Horizontal Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCheckboxHorCursorsTrqButton()
{
    checkboxStyle();
    if (ImGui::Checkbox("##Show Horizontal Cursors Trq", &cursorsHorizontalTorqueEnabled))
    {
        cursorAHorPositionTrq = minTrq;
        cursorBHorPositionTrq = maxTrq;
    }
    ImGui::SameLine();
    ImGui::Text("Horizontal Cursors");
    endCheckboxStyle();
}

void GraphicInterface::drawCursorMenu()
{
    mab::MdMode_E currentMode = m_data->currentMode;
    if (currentMode == mab::MdMode_E::IDLE)
    {
        showCursorMenu = false;
        ImGui::BeginDisabled();
    }

    ImGui::SetCursorPosX(leftMenuPadding);
    checkboxStyle();
    ImGui::Checkbox("##Show Cursor Menu", &showCursorMenu);
    ImGui::SameLine();
    ImGui::Text("Show Cursor Menu");
    endCheckboxStyle();

    if (currentMode == mab::MdMode_E::IDLE)
    {
        ImGui::EndDisabled();
    }
}

/*

Helper

*/

void GraphicInterface::drawHelper(const char* description)
{
    ImGui::SetWindowFontScale(1.2f);
    ImGui::TextDisabled("(?)");
    ImGui::SetWindowFontScale(1.0f);

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(description);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

/*

Parameters setters etc.

*/

void GraphicInterface::drawParametersVelocity()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    centerText("Velocity loop - PID tuning parameters");

    ImGui::SetCursorPosX(leftMenuPadding);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * leftMenuPadding, 0.0f);
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
        ImGui::Text("%.3f", m_data->Kp_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Ki_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Ki_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd velocity");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->integralMax_vel);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->integralMax_velSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }

    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Velocity Parameters",
                      ImVec2(leftMenuBarWidth / 2.0f, mediumButtonHeight)))
    {
        m_data->Kp_velSlider          = 0.0f;
        m_data->Ki_velSlider          = 0.0f;
        m_data->Kd_velSlider          = 0.0f;
        m_data->integralMax_velSlider = 0.0f;
    }
    endButtonStyle();
}

void GraphicInterface::drawParametersPosition()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    centerText("Position loop - PID tuning parameters");

    ImGui::SetCursorPosX(leftMenuPadding);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * leftMenuPadding, 0.0f);
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
        ImGui::Text("%.3f", m_data->Kp_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Ki position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Ki_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Ki_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd position");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Integral Windup");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->integralMax_pos);
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->integralMax_posSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }

    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Position Parameters",
                      ImVec2(leftMenuBarWidth / 2.0f, mediumButtonHeight)))
    {
        m_data->Kp_posSlider          = 0.0f;
        m_data->Ki_posSlider          = 0.0f;
        m_data->Kd_posSlider          = 0.0f;
        m_data->integralMax_posSlider = 0.0f;
    }
    endButtonStyle();
}

void GraphicInterface::drawParametersImpedance()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 3;

    centerText("Impedance PD tuner");

    ImGui::SetCursorPosX(leftMenuPadding);
    ImVec2 tableSize = ImVec2(leftMenuBarWidth - 2 * leftMenuPadding, 0.0f);
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
        ImGui::Text("%.3f", m_data->Kp_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kp_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Kd impedance");
        ImGui::TableNextColumn();
        ImGui::Text("%.3f", m_data->Kd_imp);
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        buttonColorInputFloat("##hidden_label", &m_data->Kd_impSlider, 0.0f, 0.0f, "%.3f");
        ImGui::PopID();

        ImGui::EndTable();
    }

    buttonStyle();
    ImGui::SetCursorPosX(leftMenuBarWidth / 4);
    if (ImGui::Button("Reset Impedance Parameters",
                      ImVec2(leftMenuBarWidth / 2.0f, mediumButtonHeight)))
    {
        m_data->Kp_impSlider = 0.0f;
        m_data->Kd_impSlider = 0.0f;
    }
    endButtonStyle();
}

void GraphicInterface::drawSetTargetVelocity()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Target Velocity");

    if (drawBigInputFloat("##Target Velocity",
                          &m_data->targetVelocitySlider,
                          1.0f,
                          2.0f,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s"))
    {
        m_data->targetVelocitySlider = std::clamp(
            m_data->targetVelocitySlider, -m_data->maxVelocityClamp, m_data->maxVelocityClamp);
    }
}

void GraphicInterface::drawSetTargetPosition()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Target Position");

    drawBigInputFloat("##Target Position",
                      &m_data->targetPositionSlider,
                      1.0f,
                      2.0f,
                      "%.2f",
                      leftMenuBarWidth,
                      "rad");
}

void GraphicInterface::drawSetTargetTorque()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Target Torque");

    if (drawBigInputFloat("##Target Torque",
                          &m_data->targetTorqueSlider,
                          1.0f,
                          2.0f,
                          "%.2f",
                          leftMenuBarWidth,
                          "Nm"))
    {
        m_data->targetTorqueSlider =
            std::clamp(m_data->targetTorqueSlider, -m_data->maxTorqueClamp, m_data->maxTorqueClamp);
    }
}

void GraphicInterface::drawSetTargetAcceleration()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Acceleration");

    if (drawBigInputFloat("##Acceleration",
                          &m_data->targetAccelerationSlider,
                          step,
                          step_fast,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s^2"))
    {
        m_data->targetAccelerationSlider =
            std::clamp(m_data->targetAccelerationSlider, 0.0f, m_data->maxAccelerationClamp);
    }
}

void GraphicInterface::drawSetTargetDeceleration()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Deceleration");

    if (drawBigInputFloat("##Deceleration",
                          &m_data->targetDecelerationSlider,
                          step,
                          step_fast,
                          "%.2f",
                          leftMenuBarWidth,
                          "rad/s^2"))
    {
        m_data->targetDecelerationSlider =
            std::clamp(m_data->targetDecelerationSlider, 0.0f, m_data->maxDecelerationClamp);
    }
}

void GraphicInterface::drawSetPositionWindow()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Position Window");

    drawBigInputFloat("##Position Window",
                      &m_data->positionWindowSlider,
                      step,
                      step_fast,
                      "%.2f",
                      leftMenuBarWidth,
                      "rad");
}

void GraphicInterface::drawSetVelocityWindow()
{
    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("Velocity Window");

    drawBigInputFloat("##Velocity Window",
                      &m_data->velocityWindowSlider,
                      step,
                      step_fast,
                      "%.2f",
                      leftMenuBarWidth,
                      "rad/s");
}

/*

Draw plots

*/

void GraphicInterface::updatePlotData()
{
    static bool  lastTestStarted    = false;
    static float timeInTargetWindow = 0.0f;
    static float lastHardwareTime   = 0.0f;

    bool testStarted = m_data->testStarted;

    if (testStarted && !lastTestStarted)
    {
        m_data->readData               = 0;
        timeInTargetWindow             = 0.0f;
        lastHardwareTime               = 0.0f;
        m_data->guiElapsedTime         = 0.0f;
        cursorsVerticalVelocityEnabled = false;
        cursorsVerticalPositionEnabled = false;
        cursorsVerticalTorqueEnabled   = false;

        resetCursors();
        cursorsHorizontalVelocityEnabled = false;
        cursorsHorizontalPositionEnabled = false;
        cursorsHorizontalTorqueEnabled   = false;
    }

    lastTestStarted = testStarted;

    if (testStarted)
    {
        m_data->guiElapsedTime += m_io.DeltaTime;
        uint32_t currentDataWrite = m_data->plotWriteData.load(std::memory_order_acquire);

        if (m_data->readData > currentDataWrite)
        {
            m_data->readData = 0;
        }

        while (m_data->readData != currentDataWrite)
        {
            uint32_t bufferIndex = m_data->readData % commonMemory_S::PLOT_BUFFER_SIZE;

            float time      = m_data->plotTime[bufferIndex];
            float vel       = m_data->plotVelocity[bufferIndex];
            float pos       = m_data->plotPosition[bufferIndex];
            float trq       = m_data->plotTorque[bufferIndex];
            float targetVel = m_data->plotTargetVelocity[bufferIndex];
            float targetPos = m_data->plotTargetPosition[bufferIndex];

            float dt_point   = time - lastHardwareTime;
            lastHardwareTime = time;

            minVel = m_data->minVel;
            minPos = m_data->minPos;
            minTrq = m_data->minTrq;
            maxVel = m_data->maxVel;
            maxPos = m_data->maxPos;
            maxTrq = m_data->maxTrq;

            if (vel > m_data->maxVel)
                m_data->maxVel = vel;
            if (vel < m_data->minVel)
                m_data->minVel = vel;
            if (pos > m_data->maxPos)
                m_data->maxPos = pos;
            if (pos < m_data->minPos)
                m_data->minPos = pos;
            if (trq > m_data->maxTrq)
                m_data->maxTrq = trq;
            if (trq < m_data->minTrq)
                m_data->minTrq = trq;

            bool inPosWindow = std::abs(pos - targetPos) <= m_data->positionWindow;
            bool inVelWindow = std::abs(vel - targetVel) <= m_data->velocityWindow;

            switch (m_data->currentMode)
            {
                case mab::MdMode_E::VELOCITY_PID:
                    timeInTarget(inVelWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::VELOCITY_PROFILE:
                    timeInTarget(inVelWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::POSITION_PID:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::IMPEDANCE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::POSITION_PROFILE:
                    timeInTarget(inPosWindow, timeInTargetWindow, dt_point);
                    break;
                case mab::MdMode_E::IDLE:
                    break;
                case mab::MdMode_E::RAW_TORQUE:
                    break;
                default:
                    break;
            }

            m_data->readData++;
        }

        m_data->offset = currentDataWrite % commonMemory_S::PLOT_BUFFER_SIZE;
    }
}

void GraphicInterface::timeInTarget(bool& inTimeWindow, float& timeInTargetWindow, float& dt)
{
    if (inTimeWindow)
    {
        timeInTargetWindow += dt;

        if (timeInTargetWindow >= targetHoldTime && !m_data->buttonManualTestPressed)
        {
            {
                std::lock_guard<std::mutex> lock(m_data->mtx);
                m_data->testStarted = false;
                m_data->testOngoing = false;
            }
        }
    }
    else
    {
        timeInTargetWindow = 0.0f;
    }
}

void GraphicInterface::drawVelocityPlot(ImVec2 size, ImPlotFlags plotFlag)
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);

    bool testStarted = m_data->testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (ImPlot::BeginPlot("##VelocityPlot", size, plotFlag))

    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        timeElapsedOnPlot = currentTime;

        float bottomY =
            (m_data->minVel < 0.0f) ? (m_data->minVel - 0.2f * std::abs(m_data->minVel)) : 0.0f;
        float topY = (m_data->maxVel > 0.0f) ? (m_data->maxVel + 0.2f * m_data->maxVel) : 0.0f;

        if (m_data->targetVelocity > topY)
            topY = m_data->targetVelocity + 0.2f * m_data->targetVelocity;
        if (m_data->targetVelocity < bottomY)
            bottomY = m_data->targetVelocity - 0.2f * std::abs(m_data->targetVelocity);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Velocity [rad/s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Velocity(t)", m_data->plotTime, m_data->plotVelocity, pointsCount, spec);

        if (m_data->currentMode == mab::MdMode_E::VELOCITY_PID ||
            m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE ||
            m_data->currentMode == mab::MdMode_E::POSITION_PROFILE)
        {
            ImPlot::PlotStairs(
                "Target Velocity", m_data->plotTime, m_data->plotTargetVelocity, pointsCount, spec);
        }

        plotCursors(cursorAPositionVel,
                    cursorBPositionVel,
                    cursorAHorPositionVel,
                    cursorBHorPositionVel,
                    cursorsVerticalVelocityEnabled,
                    cursorsHorizontalVelocityEnabled);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

void GraphicInterface::drawPositionPlot(ImVec2 size, ImPlotFlags plotFlag)
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);

    bool testStarted = m_data->testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (ImPlot::BeginPlot("##PositionPlot", size, plotFlag))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        float bottomY =
            (m_data->minPos < 0.0f) ? (m_data->minPos - 0.2f * std::abs(m_data->minPos)) : 0.0f;
        float topY = (m_data->maxPos > 0.0f) ? (m_data->maxPos + 0.2f * m_data->maxPos) : 0.0f;

        if (m_data->targetPosition > topY)
            topY = m_data->targetPosition + 0.2f * m_data->targetPosition;
        if (m_data->targetPosition < bottomY)
            bottomY = m_data->targetPosition - 0.2f * std::abs(m_data->targetPosition);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Position [rad]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Position(t)", m_data->plotTime, m_data->plotPosition, pointsCount, spec);
        if (m_data->currentMode == mab::MdMode_E::POSITION_PID ||
            m_data->currentMode == mab::MdMode_E::POSITION_PROFILE ||
            m_data->currentMode == mab::MdMode_E::IMPEDANCE ||
            m_data->currentMode == mab::MdMode_E::VELOCITY_PROFILE)
            ImPlot::PlotStairs(
                "Target Position", m_data->plotTime, m_data->plotTargetPosition, pointsCount, spec);

        plotCursors(cursorAPositionPos,
                    cursorBPositionPos,
                    cursorAHorPositionPos,
                    cursorBHorPositionPos,
                    cursorsVerticalPositionEnabled,
                    cursorsHorizontalPositionEnabled);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

void GraphicInterface::drawTorquePlot(ImVec2 size, ImPlotFlags plotFlag)
{
    int processedSamples = static_cast<int>(m_data->readData);
    int bufferSize       = static_cast<int>(commonMemory_S::PLOT_BUFFER_SIZE);
    int pointsCount      = (processedSamples < bufferSize) ? processedSamples : bufferSize;

    ImPlotSpec spec;
    spec.Offset      = (processedSamples < bufferSize) ? 0 : static_cast<int>(m_data->offset);
    bool testStarted = m_data->testStarted;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (ImPlot::BeginPlot("##TorquegPlot", size, plotFlag))
    {
        ImPlotCond plotCondition = testStarted ? ImPlotCond_Always : ImPlotCond_Once;
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        float currentTime =
            m_data->plotTime[(m_data->offset == 0) ? commonMemory_S::PLOT_BUFFER_SIZE - 1
                                                   : m_data->offset - 1];

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, currentTime, plotCondition);

        float bottomY =
            (m_data->minTrq < 0.0f) ? (m_data->minTrq - 0.2f * std::abs(m_data->minTrq)) : 0.0f;
        float topY = (m_data->maxTrq > 0.0f) ? (m_data->maxTrq + 0.2f * m_data->maxTrq) : 0.0f;

        if (m_data->targetTorque > topY)
            topY = m_data->targetTorque + 0.2f * m_data->targetTorque;
        if (m_data->targetTorque < bottomY)
            bottomY = m_data->targetTorque - 0.2f * std::abs(m_data->targetTorque);

        if (topY == 0.0f && bottomY == 0.0f)
        {
            topY    = marginPlot;
            bottomY = -marginPlot;
        }

        ImPlot::SetupAxis(ImAxis_Y1, "Torque [Nm]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_Y1, bottomY, topY, plotCondition);

        ImPlot::PlotLine("Torque(t)", m_data->plotTime, m_data->plotTorque, pointsCount, spec);

        if (m_data->currentMode == mab::MdMode_E::IMPEDANCE)
            ImPlot::PlotStairs(
                "Target Torque", m_data->plotTime, m_data->plotTargetTorque, pointsCount, spec);

        plotCursors(cursorAPositionVel,
                    cursorBPositionTrq,
                    cursorAHorPositionTrq,
                    cursorBHorPositionTrq,
                    cursorsVerticalTorqueEnabled,
                    cursorsHorizontalTorqueEnabled);

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor();
}

void GraphicInterface::plotCursors(
    double& xA, double& xB, double& yA, double& yB, bool& verticalCursors, bool& horizontalCursors)
{
    ImPlotRect limits   = ImPlot::GetPlotLimits();
    double     dynamicY = limits.Y.Min + (limits.Y.Max - limits.Y.Min) * 0.05;
    double     dynamicX = limits.X.Min;

    if (verticalCursors)
    {
        ImVec2 offsetXA(10.f, -10.f);
        ImVec2 offsetXB(10.f, -10.f);

        float pixelX_A = ImPlot::PlotToPixels(xA, 0).x;
        float pixelX_B = ImPlot::PlotToPixels(xB, 0).x;

        if (std::abs(pixelX_A - pixelX_B) < 70.0f)
        {
            if (xA < xB)
            {
                offsetXB.y = 15.f;
            }
            else
            {
                offsetXA.y = 15.f;
            }
        }

        ImPlotDragToolFlags flags = ImPlotDragToolFlags_NoFit;

        bool dragA = ImPlot::DragLineX(0, &xA, colA, 1.0f, flags);
        bool dragB = ImPlot::DragLineX(1, &xB, colB, 1.0f, flags);

        ImPlot::TagX(xA, colA, "tA");
        ImPlot::TagX(xB, colB, "tB");

        if (dragA || ImGui::IsItemHovered())
        {
            ImPlot::Annotation(xA, dynamicY, colA, offsetXA, true, "tA: %.3fs", xA);
        }
        if (dragB || ImGui::IsItemHovered())
        {
            ImPlot::Annotation(xB, dynamicY, colB, offsetXB, true, "tB: %.3fs", xB);
        }
    }

    if (horizontalCursors)
    {
        ImVec2 offsetYA(10.f, -10.f);
        ImVec2 offsetYB(10.f, -10.f);

        float pixelY_A = ImPlot::PlotToPixels(0, yA).y;
        float pixelY_B = ImPlot::PlotToPixels(0, yB).y;

        if (std::abs(pixelY_A - pixelY_B) < 30.0f)
        {
            if (pixelY_A < pixelY_B)
            {
                offsetYB.y = 15.f;
            }
            else
            {
                offsetYA.y = 15.f;
            }
        }

        ImPlotDragToolFlags flags = ImPlotDragToolFlags_NoFit;

        bool dragA  = ImPlot::DragLineY(0, &yA, colA, 1.0f, flags);
        bool hoverA = ImGui::IsItemHovered();
        bool dragB  = ImPlot::DragLineY(1, &yB, colB, 1.0f, flags);
        bool hoverB = ImGui::IsItemHovered();

        ImPlot::TagY(yA, colA, "yA");
        ImPlot::TagY(yB, colB, "yB");

        if (dragA || hoverA)
        {
            ImPlot::Annotation(dynamicX, yA, colA, offsetYA, true, "yA: %.3f", yA);
        }
        if (dragB || hoverB)
        {
            ImPlot::Annotation(dynamicX, yB, colB, offsetYB, true, "yB: %.3f", yB);
        }
    }
}

/*

Draw values

*/

void GraphicInterface::drawValuesVelocity()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 2;

    ImVec2 tableSize = ImVec2(0.0f, 0.0f);
    if (ImGui::BeginTable("ValueTableVelocity", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Cursor");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tA");
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAPositionVelTemp = static_cast<float>(cursorAPositionVel) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorAPositionVelTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorAPositionVel = cursorAPositionVelTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tB");
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBPositionVelTemp = static_cast<float>(cursorBPositionVel) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorBPositionVelTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorBPositionVel = cursorBPositionVelTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δt");
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaT = std::abs(cursorAPositionVel - cursorBPositionVel) * 1000;
        buttonColorInputFloat("##hidden_label", &deltaT, 0.0f, 0.0f, "%.3f ms");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("f (1/Δt)");
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float freq;
        if (deltaT != 0.0f)
            freq = std::abs(1 / deltaT) * 1000;
        buttonColorInputFloat("##hidden_label", &freq, 0.0f, 0.0f, "%.3f Hz");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yA");
        ImGui::TableNextColumn();
        ImGui::PushID(5);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAHorPositionVelTemp = static_cast<float>(cursorAHorPositionVel);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorAHorPositionVelTemp, 0.0f, 0.0f, "%.3f rad/s"))
        {
            cursorAHorPositionVel = cursorAHorPositionVelTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yB");
        ImGui::TableNextColumn();
        ImGui::PushID(6);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBHorPositionVelTemp = static_cast<float>(cursorBHorPositionVel);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorBHorPositionVelTemp, 0.0f, 0.0f, "%.3f rad/s"))
        {
            cursorBHorPositionVelTemp = cursorBHorPositionVelTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δy");
        ImGui::TableNextColumn();
        ImGui::PushID(7);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaY = std::abs(cursorAHorPositionVel - cursorBHorPositionVel);
        buttonColorInputFloat("##hidden_label", &deltaY, 0.0f, 0.0f, "%.3f rad/s");
        ImGui::PopID();

        ImGui::EndTable();
    }
}

void GraphicInterface::drawValuesPosition()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 2;

    ImVec2 tableSize = ImVec2(0.0f, 0.0f);
    if (ImGui::BeginTable("ValueTablePosition", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Cursor");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tA");
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAPositionPosTemp = static_cast<float>(cursorAPositionPos) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorAPositionPosTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorAPositionPos = cursorAPositionPosTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tB");
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBPositionPosTemp = static_cast<float>(cursorBPositionPos) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorBPositionPosTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorBPositionPos = cursorBPositionPosTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δt");
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaT = std::abs(cursorAPositionPos - cursorBPositionPos) * 1000;
        buttonColorInputFloat("##hidden_label", &deltaT, 0.0f, 0.0f, "%.3f ms");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("f (1/Δt)");
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float freq;
        if (deltaT != 0.0f)
            freq = std::abs(1 / deltaT) * 1000;
        buttonColorInputFloat("##hidden_label", &freq, 0.0f, 0.0f, "%.3f Hz");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yA");
        ImGui::TableNextColumn();
        ImGui::PushID(5);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAHorPositionPosTemp = static_cast<float>(cursorAHorPositionPos);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorAHorPositionPosTemp, 0.0f, 0.0f, "%.3f rad"))
        {
            cursorAHorPositionPos = cursorAHorPositionPosTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yB");
        ImGui::TableNextColumn();
        ImGui::PushID(6);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBHorPositionPosTemp = static_cast<float>(cursorBHorPositionPos);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorBHorPositionPosTemp, 0.0f, 0.0f, "%.3f rad"))
        {
            cursorBHorPositionPos = cursorBHorPositionPosTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δy");
        ImGui::TableNextColumn();
        ImGui::PushID(7);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaY = std::abs(cursorAHorPositionPos - cursorBHorPositionPos);
        buttonColorInputFloat("##hidden_label", &deltaY, 0.0f, 0.0f, "%.3f rad");
        ImGui::PopID();

        ImGui::EndTable();
    }
}

void GraphicInterface::drawValuesTorque()
{
    ImGuiTableFlags flags = ImGuiTableFlags_Borders;

    const uint8_t numberOfColumns = 2;

    ImVec2 tableSize = ImVec2(0.0f, 0.0f);
    if (ImGui::BeginTable("ValueTableTorque", numberOfColumns, flags, tableSize))
    {
        ImGui::TableSetupColumn("Cursor");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tA");
        ImGui::TableNextColumn();
        ImGui::PushID(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAPositionTrqTemp = static_cast<float>(cursorAPositionTrq) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorAPositionTrqTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorAPositionTrq = cursorAPositionTrqTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("tB");
        ImGui::TableNextColumn();
        ImGui::PushID(2);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBPositionTrqTemp = static_cast<float>(cursorBPositionTrq) * 1000;
        if (buttonColorInputFloat("##hidden_label", &cursorBPositionTrqTemp, 0.0f, 0.0f, "%.3f ms"))
        {
            cursorBPositionTrq = cursorBPositionTrqTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δt");
        ImGui::TableNextColumn();
        ImGui::PushID(3);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaT = std::abs(cursorAPositionTrq - cursorBPositionTrq) * 1000;
        buttonColorInputFloat("##hidden_label", &deltaT, 0.0f, 0.0f, "%.3f ms");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("f (1/Δt)");
        ImGui::TableNextColumn();
        ImGui::PushID(4);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float freq;
        if (deltaT != 0.0f)
            freq = std::abs(1 / deltaT) * 1000;
        buttonColorInputFloat("##hidden_label", &freq, 0.0f, 0.0f, "%.3f Hz");
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yA");
        ImGui::TableNextColumn();
        ImGui::PushID(5);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorAHorPositionTrqTemp = static_cast<float>(cursorAHorPositionTrq);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorAHorPositionTrqTemp, 0.0f, 0.0f, "%.3f Nm"))
        {
            cursorAHorPositionTrq = cursorAHorPositionTrqTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("yB");
        ImGui::TableNextColumn();
        ImGui::PushID(6);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float cursorBHorPositionTrqTemp = static_cast<float>(cursorBHorPositionTrq);
        if (buttonColorInputFloat(
                "##hidden_label", &cursorBHorPositionTrqTemp, 0.0f, 0.0f, "%.3f Nm"))
        {
            cursorBHorPositionTrq = cursorBHorPositionTrqTemp;
        }
        ImGui::PopID();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Δy");
        ImGui::TableNextColumn();
        ImGui::PushID(7);
        ImGui::SetNextItemWidth(-FLT_MIN);
        float deltaY = std::abs(cursorAHorPositionTrq - cursorBHorPositionTrq);
        buttonColorInputFloat("##hidden_label", &deltaY, 0.0f, 0.0f, "%.3f Nm");
        ImGui::PopID();

        ImGui::EndTable();
    }
}

/*

Style

*/

void GraphicInterface::comboStyle(const char* text)
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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));

    ImVec2 currentPadding = ImGui::GetStyle().FramePadding;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(currentPadding.x, roundingFrameButton));

    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::Text("%s", text);

    ImGui::SetCursorPosX(leftMenuPadding);
    ImGui::SetNextItemWidth(leftMenuBarWidth - 2 * leftMenuPadding);
}

void GraphicInterface::endComboStyle()
{
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(7);
}

void GraphicInterface::buttonStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, mabColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
}

void GraphicInterface::buttonSelectStyle(bool flag)
{
    ImVec4 colorNormal, colorHovered, colorActive;

    if (flag)
    {
        colorNormal  = buttonColor;
        colorHovered = mabColorHovered;
        colorActive  = buttonColor;
    }
    else
    {
        colorNormal  = clear_color;
        colorHovered = mabColorHovered;
        colorActive  = mabColor;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameButton);

    ImGui::PushStyleColor(ImGuiCol_Button, colorNormal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorActive);
}

void GraphicInterface::endButtonStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void GraphicInterface::endButtonSelectStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}

void GraphicInterface::buttonImportantStyle(bool& flag)
{
    ImVec4 colorNormal, colorHovered, colorActive;
    float  borderSize;

    if (flag)
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
}

void GraphicInterface::endButtonImportantStyle()
{
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
}

void GraphicInterface::checkboxStyle()
{
    ImGui::PushStyleColor(ImGuiCol_CheckMark, mabColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, mabColorHovered);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, roundingFrameCheckbox);
}

void GraphicInterface::endCheckboxStyle()
{
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
}

void GraphicInterface::centerText(const char* text)
{
    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth   = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text);
}

const char* GraphicInterface::getModeName(mab::MdMode_E mode)
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

bool GraphicInterface::drawBigInputFloat(const char* label,
                                         float*      v,
                                         float       step,
                                         float       step_fast,
                                         const char* format,
                                         float       windowWidth,
                                         const char* unit)
{
    bool valueChanged = false;

    std::string formatStr = format;
    if (unit != nullptr && unit[0] != '\0')
    {
        formatStr += " ";
        formatStr += unit;
    }

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
    if (ImGui::InputFloat("##input", v, 0.0f, 0.0f, formatStr.c_str(), ImGuiInputTextFlags_None))
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

bool GraphicInterface::buttonColorInputFloat(const char* label,
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
